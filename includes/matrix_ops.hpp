#pragma once

#include "onn.hpp"
#include "matrix.hpp"

void matmul       (const Matrix& A, const Matrix& B, Matrix& out);  // out = A · B     (m*k)(k*n) -> m*n
void matmulTransA (const Matrix& A, const Matrix& B, Matrix& out);  // out = Aᵀ · B    (k*m)(k*n) -> m*n
void matmulTransB (const Matrix& A, const Matrix& B, Matrix& out);  // out = A · Bᵀ    (m*k)(n*k) -> m*n

void addRowBias(Matrix& M, const Matrix& bias);          // M(i,j) += bias(0,j), in place; bias is n*1 or 1*n
void sumRows   (const Matrix& M, Matrix& out);           // out(0,j) = sum over i of M(i,j); out is 1*n

void hadamard(const Matrix& A, const Matrix& B, Matrix& out);
void subtract(const Matrix& A, const Matrix& B, Matrix& out);
void scale   (Matrix& A, double s);
