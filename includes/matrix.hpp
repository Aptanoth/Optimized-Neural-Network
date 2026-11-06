#pragma once

#include "onn.hpp"

class Matrix {
    size_t rows_ = 0;
    size_t cols_ = 0;
    std::vector<double> data_;   // row-major: element (i, j) is data_[i * cols_ + j]

public:
    Matrix() = default;
    Matrix(size_t rows, size_t cols, double value = 0.0)
        :rows_(rows), cols_(cols), data_(std::vector<double>(rows * cols, value)) {}
    Matrix(size_t rows, size_t cols, std::initializer_list<double> values) // for tests
        : rows_(rows), cols_(cols), data_(values) {
            if (values.size() != rows * cols)
                throw std::invalid_argument("Matrix: initializer list size doesn't match rows*cols");
        }

    size_t rows() const { return rows_; }
    size_t cols() const { return cols_; }
    size_t size() const { return rows_ * cols_;}

    double& operator()(size_t i, size_t j)       { return data_[i * cols_ + j]; }
    double  operator()(size_t i, size_t j) const { return data_[i * cols_ + j]; }

    double*       data()       { return data_.data(); }
    const double* data() const { return data_.data(); }

    void resize(size_t rows, size_t cols);
    void fill(double value);
    void zero() { fill(0.0); }
};
