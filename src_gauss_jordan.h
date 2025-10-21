#ifndef SRC_GAUSS_JORDAN_H
#define SRC_GAUSS_JORDAN_H

#include <vector>
#include <iostream>

// Matrix stores an augmented matrix of size n x (n+1) representing [A|b]
class Matrix {
public:
    int n;
    std::vector<std::vector<double>> data; // n rows, n+1 columns

    Matrix(int size = 0);
    void print() const;
    void fillRandom(double low = -10.0, double high = 10.0);
};

// Sequential Gauss-Jordan (existing single-threaded algorithm)
void gaussJordanSequential(Matrix &matrix);

// Parallel Gauss-Jordan: threadCount = number of worker threads to use (>=1)
// The function assumes matrix is a valid augmented matrix n x (n+1).
void gaussJordanParallel(Matrix &matrix, unsigned threadCount = 0);

// Compute residual norm ||Ax - b||_2 for the original A and solution in the last column
double residualNorm(const Matrix &orig, const Matrix &reduced);

#endif // SRC_GAUSS_JORDAN_H