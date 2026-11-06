#include "matrix.hpp"


void Matrix::resize(size_t rows, size_t cols){
    if (rows * cols > data_.size())
        data_.resize(rows * cols);
    rows_ = rows;
    cols_ = cols;
}

void Matrix::fill(double value){
    for (size_t i = 0; i < rows_ * cols_; i++)
        data_[i] = value;
}
