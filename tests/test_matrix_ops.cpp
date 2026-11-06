#include <iostream>
#include <cmath>
#include <stdexcept>
#include "matrix.hpp"
#include "matrix_ops.hpp"

// Run with:  make test T=tests/test_matrix_ops.cpp          (add SAN=1 for sanitizers)
//
// Signatures assumed (from our spec):
//   void matmul      (const Matrix& A, const Matrix& B, Matrix& out);   // A * B
//   void matmulTransA(const Matrix& A, const Matrix& B, Matrix& out);   // A^T * B
//   void matmulTransB(const Matrix& A, const Matrix& B, Matrix& out);   // A * B^T
//   void addRowBias  (Matrix& M, const Matrix& bias);                   // in place, bias is 1 x n
//   void sumRows     (const Matrix& M, Matrix& out);                    // out is 1 x n
//   void hadamard    (const Matrix& A, const Matrix& B, Matrix& out);
//   void subtract    (const Matrix& A, const Matrix& B, Matrix& out);
//   void scale       (Matrix& M, double s);                             // in place
// If your scale (or anything else) has a different signature, adjust the matching calls below.

static int failures = 0;

void check(bool cond, const char* name) {
    std::cout << (cond ? "[PASS] " : "[FAIL] ") << name << "\n";
    if (!cond) failures++;
}

void section(const char* title) { std::cout << "\n--- " << title << " ---\n"; }

bool near(double a, double b) { return std::fabs(a - b) < 1e-9; }

bool sameMatrix(const Matrix& a, const Matrix& b) {
    if (a.rows() != b.rows() || a.cols() != b.cols()) return false;
    for (size_t i = 0; i < a.rows(); ++i)
        for (size_t j = 0; j < a.cols(); ++j)
            if (!near(a(i, j), b(i, j))) return false;
    return true;
}

template <class F>
bool throwsInvalid(F f) {
    try {
        f();
    } catch (const std::invalid_argument&) {
        return true;
    } catch (...) {
        return false;
    }
    return false;
}

// ---- reference helpers (deliberately naive) ----
Matrix transposed(const Matrix& m) {
    Matrix t(m.cols(), m.rows());
    for (size_t i = 0; i < m.rows(); ++i)
        for (size_t j = 0; j < m.cols(); ++j)
            t(j, i) = m(i, j);
    return t;
}

Matrix refMatmul(const Matrix& A, const Matrix& B) {
    Matrix out(A.rows(), B.cols());
    for (size_t i = 0; i < A.rows(); ++i)
        for (size_t j = 0; j < B.cols(); ++j) {
            double s = 0;
            for (size_t k = 0; k < A.cols(); ++k) s += A(i, k) * B(k, j);
            out(i, j) = s;
        }
    return out;
}

Matrix pattern(size_t r, size_t c, double seed) {
    Matrix m(r, c);
    for (size_t i = 0; i < r; ++i)
        for (size_t j = 0; j < c; ++j)
            m(i, j) = std::sin(seed + 1.7 * i + 0.9 * j) + 0.1 * i - 0.05 * j;
    return m;
}

// ---------------------------------------------------------------- matmul
void testMatmul() {
    section("matmul  (A * B)");

    Matrix A(2, 3, {1, 2, 3, 4, 5, 6});
    Matrix B(3, 2, {7, 8, 9, 10, 11, 12});
    Matrix expected(2, 2, {58, 64, 139, 154});

    Matrix out;
    matmul(A, B, out);
    check(sameMatrix(out, expected), "2x3 * 3x2 == {58,64,139,154}");
    check(sameMatrix(A, Matrix(2, 3, {1, 2, 3, 4, 5, 6})) && sameMatrix(B, Matrix(3, 2, {7, 8, 9, 10, 11, 12})),
          "inputs are unchanged");

    matmul(A, B, out);
    check(sameMatrix(out, expected), "calling twice into the same out does not accumulate");

    Matrix junk(5, 5, 99.0);
    matmul(A, B, junk);
    check(sameMatrix(junk, expected), "out with wrong shape and junk values is resized and overwritten");

    Matrix row(1, 3, {1, 2, 3}), col(3, 1, {4, 5, 6});
    Matrix dotOut;
    matmul(row, col, dotOut);
    check(dotOut.rows() == 1 && dotOut.cols() == 1 && near(dotOut(0, 0), 32), "1x3 * 3x1 == dot product 32");

    Matrix outerOut;
    matmul(col, row, outerOut);
    check(sameMatrix(outerOut, Matrix(3, 3, {4, 8, 12, 5, 10, 15, 6, 12, 18})), "3x1 * 1x3 == outer product");

    Matrix I(3, 3, {1, 0, 0, 0, 1, 0, 0, 0, 1});
    Matrix AI;
    matmul(A, I, AI);
    check(sameMatrix(AI, A), "A * identity == A");

    check(throwsInvalid([&] { Matrix o; matmul(A, A, o); }), "shape mismatch (2x3 * 2x3) throws");
}

// ---------------------------------------------------------------- matmulTransA
void testMatmulTransA() {
    section("matmulTransA  (A^T * B)");

    Matrix At(3, 2, {1, 4, 2, 5, 3, 6});       // transpose of {1,2,3,4,5,6} (2x3)
    Matrix B(3, 2, {7, 8, 9, 10, 11, 12});
    Matrix expected(2, 2, {58, 64, 139, 154});

    Matrix out;
    matmulTransA(At, B, out);
    check(sameMatrix(out, expected), "At(3x2)^T * B(3x2) == {58,64,139,154}");

    matmulTransA(At, B, out);
    check(sameMatrix(out, expected), "calling twice into the same out does not accumulate");

    Matrix junk(5, 5, 99.0);
    matmulTransA(At, B, junk);
    check(sameMatrix(junk, expected), "junk out is resized and overwritten");

    check(throwsInvalid([&] { Matrix o; matmulTransA(At, Matrix(2, 2), o); }),
          "shape mismatch (row counts differ) throws");
}

// ---------------------------------------------------------------- matmulTransB
void testMatmulTransB() {
    section("matmulTransB  (A * B^T)");

    Matrix A(2, 3, {1, 2, 3, 4, 5, 6});
    Matrix Bt(2, 3, {7, 9, 11, 8, 10, 12});    // transpose of {7,8,9,10,11,12} (3x2)
    Matrix expected(2, 2, {58, 64, 139, 154});

    Matrix out;
    matmulTransB(A, Bt, out);
    check(sameMatrix(out, expected), "A(2x3) * Bt(2x3)^T == {58,64,139,154}");

    matmulTransB(A, Bt, out);
    check(sameMatrix(out, expected), "calling twice into the same out is stable");

    Matrix junk(5, 5, 99.0);
    matmulTransB(A, Bt, junk);
    check(sameMatrix(junk, expected), "junk out is resized and overwritten");

    check(throwsInvalid([&] { Matrix o; matmulTransB(A, Matrix(2, 2), o); }),
          "shape mismatch (column counts differ) throws");
}

// ---------------------------------------------------------------- cross checks
void testCrossChecks() {
    section("cross-checks against a naive reference (non-trivial shapes)");

    Matrix A = pattern(4, 5, 0.3);
    Matrix B = pattern(5, 3, 1.1);
    Matrix ref = refMatmul(A, B);

    Matrix out;
    matmul(A, B, out);
    check(sameMatrix(out, ref), "matmul matches the naive reference (4x5 * 5x3)");

    matmulTransA(transposed(A), B, out);
    check(sameMatrix(out, ref), "matmulTransA(A^T, B) matches the reference");

    matmulTransB(A, transposed(B), out);
    check(sameMatrix(out, ref), "matmulTransB(A, B^T) matches the reference");

    Matrix C = pattern(7, 1, 2.0), D = pattern(1, 6, 0.7);
    matmul(C, D, out);
    check(sameMatrix(out, refMatmul(C, D)), "matmul 7x1 * 1x6 matches the reference");

    Matrix E = pattern(1, 9, 0.2), F = pattern(9, 1, 0.5);
    matmul(E, F, out);
    check(sameMatrix(out, refMatmul(E, F)), "matmul 1x9 * 9x1 matches the reference");
}

// ---------------------------------------------------------------- bias / sumRows
void testBiasAndSum() {
    section("addRowBias and sumRows");

    Matrix M(2, 2, {1, 2, 3, 4});
    Matrix bias(1, 2, {10, 20});
    addRowBias(M, bias);
    check(sameMatrix(M, Matrix(2, 2, {11, 22, 13, 24})), "addRowBias adds the bias to every row");
    check(sameMatrix(bias, Matrix(1, 2, {10, 20})), "bias is unchanged");

    check(throwsInvalid([&] { Matrix X(2, 2); addRowBias(X, Matrix(1, 3)); }), "addRowBias: wrong bias width throws");
    check(throwsInvalid([&] { Matrix X(2, 2); addRowBias(X, Matrix(2, 2)); }), "addRowBias: bias with 2 rows throws");

    Matrix S(3, 2, {1, 2, 3, 4, 5, 6});
    Matrix out;
    sumRows(S, out);
    check(sameMatrix(out, Matrix(1, 2, {9, 12})), "sumRows({1,2;3,4;5,6}) == {9,12} (1x2)");

    sumRows(S, out);
    check(sameMatrix(out, Matrix(1, 2, {9, 12})), "sumRows twice into the same out does not accumulate");

    Matrix junk(4, 4, 99.0);
    sumRows(S, junk);
    check(sameMatrix(junk, Matrix(1, 2, {9, 12})), "sumRows with a junk out is resized and overwritten");

    Matrix single(1, 3, {1, 2, 3});
    sumRows(single, out);
    check(sameMatrix(out, single), "sumRows of a single row is that row");
}

// ---------------------------------------------------------------- elementwise
void testElementwise() {
    section("hadamard, subtract, scale");

    Matrix A(2, 2, {1, 2, 3, 4}), B(2, 2, {5, 6, 7, 8});

    Matrix out;
    hadamard(A, B, out);
    check(sameMatrix(out, Matrix(2, 2, {5, 12, 21, 32})), "hadamard == {5,12,21,32}");
    check(sameMatrix(A, Matrix(2, 2, {1, 2, 3, 4})) && sameMatrix(B, Matrix(2, 2, {5, 6, 7, 8})),
          "hadamard leaves inputs unchanged");

    Matrix A2 = A;
    hadamard(A2, B, A2);
    check(sameMatrix(A2, Matrix(2, 2, {5, 12, 21, 32})), "hadamard with out aliasing an input");

    Matrix junk(3, 3, 99.0);
    hadamard(A, B, junk);
    check(sameMatrix(junk, Matrix(2, 2, {5, 12, 21, 32})), "hadamard with a junk out is resized");

    check(throwsInvalid([&] { Matrix o; hadamard(A, Matrix(2, 3), o); }), "hadamard shape mismatch throws");

    Matrix P(2, 2, {5, 7, 9, 11}), Q(2, 2, {1, 2, 3, 4});
    subtract(P, Q, out);
    check(sameMatrix(out, Matrix(2, 2, {4, 5, 6, 7})), "subtract == {4,5,6,7}");

    Matrix P2 = P;
    subtract(P2, Q, P2);
    check(sameMatrix(P2, Matrix(2, 2, {4, 5, 6, 7})), "subtract with out aliasing the first input");

    check(sameMatrix(Q, Matrix(2, 2, {1, 2, 3, 4})), "subtract leaves the second input unchanged");
    check(throwsInvalid([&] { Matrix o; subtract(P, Matrix(3, 2), o); }), "subtract shape mismatch throws");

    Matrix S(2, 2, {1, 2, 3, 4});
    scale(S, 2.0);
    check(sameMatrix(S, Matrix(2, 2, {2, 4, 6, 8})), "scale by 2 (in place)");
    scale(S, 0.0);
    check(sameMatrix(S, Matrix(2, 2, {0, 0, 0, 0})), "scale by 0 gives zeros");
    Matrix N(1, 3, {1, -2, 3});
    scale(N, -1.0);
    check(sameMatrix(N, Matrix(1, 3, {-1, 2, -3})), "scale by -1 flips signs");
}

int main() {
    testMatmul();
    testMatmulTransA();
    testMatmulTransB();
    testCrossChecks();
    testBiasAndSum();
    testElementwise();

    std::cout << "\n" << (failures == 0 ? "All tests passed." : "Some tests failed.")
              << " (" << failures << " failures)\n";
    return failures;
}
