#include "matrix_ops.hpp"

void matmul(const Matrix& A, const Matrix& B, Matrix& out){
    if (A.cols() != B.rows())
        throw std::invalid_argument("Matrix: cannot multiply matrixes of incompatible dimensions");

    out.resize(A.rows(), B.cols());

    for (size_t i = 0; i < out.rows(); i++){
        for(size_t j = 0; j < out.cols(); j++){
            out(i, j) = 0;
            for (size_t k = 0; k < A.cols(); k++)
                out(i, j) += A(i, k) * B(k, j);
        }
    }
}

void matmulTransA(const Matrix& A, const Matrix& B, Matrix& out){
    if (A.rows() != B.rows())
        throw std::invalid_argument("Matrix: cannot multiply matrixes of incompatible dimensions");

    out.resize(A.cols(), B.cols());

    for (size_t i = 0; i < out.rows(); i++){
        for(size_t j = 0; j < out.cols(); j++){
            out(i, j) = 0;
            for (size_t k = 0; k < A.rows(); k++)
                out(i, j) += A(k, i) * B(k, j);
        }
    }
}

void matmulTransB(const Matrix& A, const Matrix& B, Matrix& out){
    if (A.cols() != B.cols())
        throw std::invalid_argument("Matrix: cannot multiply matrixes of incompatible dimensions");

    out.resize(A.rows(), B.rows());

    for (size_t i = 0; i < out.rows(); i++){
        for(size_t j = 0; j < out.cols(); j++){
            out(i, j) = 0;
            for (size_t k = 0; k < A.cols(); k++)
                out(i, j) += A(i, k) * B(j, k);
        }
    }
}

void addRowBias(Matrix& M, const Matrix& bias){
    if ((bias.rows() != 1 && bias.cols() != 1) || (bias.rows() != M.rows() && bias.cols() != M.rows()))
        throw std::invalid_argument("Matrix: Bias must be of length M.rows");

    for (size_t i = 0; i < M.rows(); i++)
        for (size_t j = 0; j < M.cols(); j++)
            M(i, j) += bias.data()[i];
}

void sumRows(const Matrix& M, Matrix& out){
    out.resize(1, M.rows());
    out.zero();

    for (size_t i = 0; i < M.rows(); i++)
        for (size_t j = 0; j < M.cols(); j++)
            out(i, 0) += M(i, j);
}

void hadamard(const Matrix& A, const Matrix& B, Matrix& out){
    if (A.rows() != B.rows() || A.cols() != B.cols())
        throw std::invalid_argument("Matrix: hadamard product must take same size vectors");

    size_t full_len = A.rows() * A.cols();
    out.resize(A.rows(), A.cols());

    for (size_t i = 0; i < full_len; i++)
        out.data()[i] = A.data()[i] * B.data()[i];
}

void subtract(const Matrix& A, const Matrix& B, Matrix& out){
    if (A.rows() != B.rows() || A.cols() != B.cols())
        throw std::invalid_argument("Matrix: hadamard product must take same size vectors");

    size_t full_len = A.rows() * A.cols();
    out.resize(A.rows(), A.cols());

    for (size_t i = 0; i < full_len; i++)
        out.data()[i] = A.data()[i] - B.data()[i];
}

void scale(Matrix& M, double s){
    size_t full_len = M.rows() * M.cols();

    for (size_t i = 0; i < full_len; i++)
        M.data()[i] *= s;
}
