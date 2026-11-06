#include <iostream>
#include <cmath>
#include <stdexcept>
#include "matrix.hpp"

// Build: g++ -std=c++17 -Wall -Wextra -Iincludes tests/test_matrix.cpp src/matrix.cpp -o test_matrix && ./test_matrix
// (adjust the paths to your layout)

static int failures = 0;

void check(bool cond, const char* name) {
    std::cout << (cond ? "[PASS] " : "[FAIL] ") << name << "\n";
    if (!cond) failures++;
}

void section(const char* title) { std::cout << "\n--- " << title << " ---\n"; }

bool near(double a, double b) { return std::fabs(a - b) < 1e-9; }

// every element of m (rows*cols of them) equals value
bool allEqual(const Matrix& m, double value) {
    for (size_t i = 0; i < m.rows(); ++i)
        for (size_t j = 0; j < m.cols(); ++j)
            if (!near(m(i, j), value)) return false;
    return true;
}

bool sameMatrix(const Matrix& a, const Matrix& b) {
    if (a.rows() != b.rows() || a.cols() != b.cols()) return false;
    for (size_t i = 0; i < a.rows(); ++i)
        for (size_t j = 0; j < a.cols(); ++j)
            if (!near(a(i, j), b(i, j))) return false;
    return true;
}

void testConstruction() {
    section("construction");

    Matrix empty;
    check(empty.rows() == 0 && empty.cols() == 0 && empty.size() == 0, "default constructor is 0x0, size 0");

    Matrix z(2, 3);
    check(z.rows() == 2 && z.cols() == 3, "Matrix(2,3) has shape 2x3");
    check(z.size() == 6, "Matrix(2,3) has size 6");
    check(allEqual(z, 0.0), "Matrix(2,3) is zero-filled");

    Matrix s(2, 3, 7.0);
    check(s.size() == 6 && allEqual(s, 7.0), "Matrix(2,3,7.0) is filled with 7");

    Matrix t(3, 1, 2.5);
    check(t.rows() == 3 && t.cols() == 1 && allEqual(t, 2.5), "column matrix 3x1 filled with 2.5");

    Matrix l(2, 3, {1, 2, 3, 4, 5, 6});
    check(l.rows() == 2 && l.cols() == 3 && l.size() == 6, "initializer-list constructor shape");
    check(near(l(0, 0), 1) && near(l(0, 1), 2) && near(l(0, 2), 3), "initializer list fills row 0 first");
    check(near(l(1, 0), 4) && near(l(1, 1), 5) && near(l(1, 2), 6), "initializer list fills row 1 next (row-major)");

    bool threw = false;
    try {
        Matrix bad(2, 3, {1, 2});
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    check(threw, "initializer list with wrong element count throws invalid_argument");
}

void testAccess() {
    section("element access and layout");

    Matrix m(2, 3);
    m(1, 1) = 42;
    check(near(m(1, 1), 42), "write then read m(1,1)");
    int others = 0;
    for (size_t i = 0; i < 2; ++i)
        for (size_t j = 0; j < 3; ++j)
            if (!(i == 1 && j == 1) && !near(m(i, j), 0)) others++;
    check(others == 0, "writing m(1,1) changes only that element");

    const Matrix c(2, 2, {1, 2, 3, 4});
    check(near(c(0, 1), 2) && near(c(1, 0), 3), "const Matrix can be read");

    Matrix r(2, 3, {1, 2, 3, 4, 5, 6});
    check(near(r.data()[0], 1) && near(r.data()[2], 3) && near(r.data()[3], 4) && near(r.data()[5], 6),
          "data() is row-major: element (i,j) at i*cols + j");
    r.data()[1 * 3 + 2] = 99;
    check(near(r(1, 2), 99), "writing through data() is visible through operator()");

    Matrix copy = r;
    copy(0, 0) = -1;
    check(near(r(0, 0), 1) && near(copy(0, 0), -1), "copy is independent of the original");
}

void testResize() {
    section("resize");

    Matrix m(2, 3, {1, 2, 3, 4, 5, 6});

    m.resize(3, 2);
    check(m.rows() == 3 && m.cols() == 2 && m.size() == 6, "resize(3,2) on 2x3: shape 3x2, size 6");

    // grow: write to EVERY element, then read back (catches a vector that wasn't actually enlarged)
    m.resize(4, 5);
    check(m.rows() == 4 && m.cols() == 5 && m.size() == 20, "grow to 4x5: shape and size 20");
    for (size_t i = 0; i < 4; ++i)
        for (size_t j = 0; j < 5; ++j)
            m(i, j) = static_cast<double>(i * 10 + j);
    bool ok = true;
    for (size_t i = 0; i < 4; ++i)
        for (size_t j = 0; j < 5; ++j)
            if (!near(m(i, j), static_cast<double>(i * 10 + j))) ok = false;
    check(ok, "all 20 elements are writable and readable after growing");

    // shrink
    m.resize(1, 2);
    check(m.rows() == 1 && m.cols() == 2, "shrink to 1x2: shape");
    check(m.size() == 2, "shrink to 1x2: size() == rows*cols == 2");

    // buffer reuse: shrinking then growing back must not reallocate
    Matrix big(2, 3);
    const double* p = big.data();
    big.resize(1, 2);
    big.resize(2, 3);
    check(big.data() == p, "shrink then grow back keeps the same buffer (no reallocation)");

    big.resize(2, 3);
    check(big.data() == p, "resize to the same shape keeps the same buffer");

    Matrix e;
    e.resize(2, 2);
    e(1, 1) = 3;
    check(e.size() == 4 && near(e(1, 1), 3), "resize on a default-constructed matrix works");
}

void testFill() {
    section("fill and zero");

    Matrix m(2, 3);
    m.fill(5.0);
    check(allEqual(m, 5.0), "fill(5) sets every element to 5 (not 0)");

    m.fill(-1.5);
    check(allEqual(m, -1.5), "fill(-1.5) sets every element to -1.5");

    m.zero();
    check(allEqual(m, 0.0), "zero() sets every element to 0");

    Matrix other(2, 3, {1, 2, 3, 4, 5, 6});
    Matrix expected(2, 3, {1, 2, 3, 4, 5, 6});
    check(sameMatrix(other, expected), "sameMatrix helper agrees on equal matrices");
    other(0, 0) = 0;
    check(!sameMatrix(other, expected), "sameMatrix helper detects a difference");
}

int main() {
    testConstruction();
    testAccess();
    testResize();
    testFill();

    std::cout << "\n" << (failures == 0 ? "All tests passed." : "Some tests failed.")
              << " (" << failures << " failures)\n";
    return failures;
}
