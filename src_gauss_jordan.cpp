#include "src_gauss_jordan.h"
#include <random>
#include <iomanip>

Matrix::Matrix(int size) : n(size) {
    data.resize(n, std::vector<double>(n + 1, 0.0));
}

void Matrix::print() const {
    for (const auto &row : data) {
        for (double val : row)
            std::cout << std::setw(10) << val << " ";
        std::cout << "\n";
    }
    std::cout << std::endl;
}

void Matrix::fillRandom() {
    std::mt19937 gen(std::random_device{}());
    std::uniform_real_distribution<double> dist(-10.0, 10.0);

    for (auto &row : data)
        for (double &val : row)
            val = dist(gen);
}

void Matrix::gaussJordanElimination() {
    for (int i = 0; i < n; ++i) {
        double pivot = data[i][i];
        for (int j = 0; j <= n; ++j)
            data[i][j] /= pivot;

        for (int k = 0; k < n; ++k) {
            if (k == i) continue;
            double factor = data[k][i];
            for (int j = 0; j <= n; ++j)
                data[k][j] -= factor * data[i][j];
        }
    }
}
