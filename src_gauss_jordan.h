#ifndef SRC_GAUSS_JORDAN_H
#define SRC_GAUSS_JORDAN_H

#include <vector>
#include <iostream>

class Matrix {
private:
    int n;
    std::vector<std::vector<double>> data;

public:
    Matrix(int size);
    void print() const;
    void fillRandom();
    void gaussJordanElimination();
};

#endif
