#include "src_gauss_jordan.h"
#include <random>
#include <iomanip>
#include <cmath>
#include <thread>
#include <algorithm>
#include <stdexcept>

Matrix::Matrix(int size) : n(size) {
    data.assign(n, std::vector<double>(n + 1, 0.0));
}

void Matrix::print() const {
    for (const auto &row : data) {
        for (double val : row)
            std::cout << std::setw(12) << std::setprecision(6) << val << " ";
        std::cout << "\n";
    }
    std::cout << std::endl;
}

void Matrix::fillRandom(double low, double high) {
    std::mt19937_64 gen(std::random_device{}());
    std::uniform_real_distribution<double> dist(low, high);

    for (int i = 0; i < n; ++i) {
        double row_abs_sum = 0.0;
        for (int j = 0; j <= n; ++j) {
            data[i][j] = dist(gen);
            if (j < n) row_abs_sum += std::fabs(data[i][j]);
        }
        // To reduce chance of singular matrix, enforce diagonal dominance:
        // add row_abs_sum + 1 to diagonal A[i][i]
        data[i][i] += (row_abs_sum + 1.0);
    }
}

// Helper: swap rows i and j
static void swap_rows(Matrix &m, int i, int j) {
    std::swap(m.data[i], m.data[j]);
}

// Sequential Gauss-Jordan with partial pivoting
void gaussJordanSequential(Matrix &matrix) {
    int n = matrix.n;
    if (n == 0) return;

    for (int k = 0; k < n; ++k) {
        // partial pivot: find max abs value in column k among rows k..n-1
        int pivot_row = k;
        double maxval = std::abs(matrix.data[k][k]);
        for (int i = k + 1; i < n; ++i) {
            double v = std::abs(matrix.data[i][k]);
            if (v > maxval) {
                maxval = v;
                pivot_row = i;
            }
        }
        if (maxval < 1e-15) {
            throw std::runtime_error("Matrix is singular or nearly singular (pivot ~ 0).");
        }
        if (pivot_row != k) swap_rows(matrix, k, pivot_row);

        // scale pivot row so that pivot becomes 1
        double pivot = matrix.data[k][k];
        for (int j = 0; j <= n; ++j) matrix.data[k][j] /= pivot;

        // eliminate other rows
        for (int i = 0; i < n; ++i) {
            if (i == k) continue;
            double factor = matrix.data[i][k];
            if (factor == 0.0) continue;
            for (int j = 0; j <= n; ++j) {
                matrix.data[i][j] -= factor * matrix.data[k][j];
            }
            // numerically force zero
            matrix.data[i][k] = 0.0;
        }
    }
}

// Parallel Gauss-Jordan with std::thread
// Approach:
//  - For each pivot k:
//      * do partial pivoting sequentially and swap rows if needed
//      * scale pivot row sequentially (so pivot row is ready for use)
//      * spawn threads, each thread processes a disjoint set of rows (excluding pivot row):
//          for its rows i: factor = A[i][k]; A[i][j] -= factor * A[k][j] for j=0..n
//      * join threads and move to next pivot
//
void gaussJordanParallel(Matrix &matrix, unsigned threadCount) {
    int n = matrix.n;
    if (n == 0) return;

    if (threadCount == 0) {
        unsigned hw = std::thread::hardware_concurrency();
        threadCount = hw > 0 ? hw : 1;
    }
    if (threadCount < 1) threadCount = 1;

    // limit threadCount to at most n (no point having more threads than rows)
    if (threadCount > static_cast<unsigned>(n)) threadCount = static_cast<unsigned>(n);

    for (int k = 0; k < n; ++k) {
        // partial pivoting (sequential)
        int pivot_row = k;
        double maxval = std::abs(matrix.data[k][k]);
        for (int i = k + 1; i < n; ++i) {
            double v = std::abs(matrix.data[i][k]);
            if (v > maxval) {
                maxval = v;
                pivot_row = i;
            }
        }
        if (maxval < 1e-15) {
            throw std::runtime_error("Matrix is singular or nearly singular (pivot ~ 0).");
        }
        if (pivot_row != k) swap_rows(matrix, k, pivot_row);

        // scale pivot row
        double pivot = matrix.data[k][k];
        for (int j = 0; j <= n; ++j) matrix.data[k][j] /= pivot;

        // prepare worker function
        auto worker = [&matrix, k, n](int start_row, int end_row) {
            // Each thread modifies its own subset of rows [start_row, end_row)
            for (int i = start_row; i < end_row; ++i) {
                if (i == k) continue; // pivot row skipped
                double factor = matrix.data[i][k];
                if (factor == 0.0) continue;
                // update row i using pivot row k
                for (int j = 0; j <= n; ++j) {
                    matrix.data[i][j] -= factor * matrix.data[k][j];
                }
                // numerically force the column to zero
                matrix.data[i][k] = 0.0;
            }
        };

        // spawn threads to work on rows [0..n)
        std::vector<std::thread> threads;
        threads.reserve(threadCount);

        // We will partition rows into roughly equal chunks, but skipping pivot row inside worker
        int rows = n;
        int base_chunk = rows / static_cast<int>(threadCount);
        int remainder = rows % static_cast<int>(threadCount);
        int cur = 0;
        for (unsigned t = 0; t < threadCount; ++t) {
            int chunk = base_chunk + (t < static_cast<unsigned>(remainder) ? 1 : 0);
            int start = cur;
            int end = cur + chunk;
            cur = end;
            // If start==end we can skip creating a thread
            if (start >= end) continue;
            threads.emplace_back(worker, start, end);
        }

        // join threads
        for (auto &th : threads) th.join();
    }
}

// residual: compute ||A*x - b||_2 where x is stored in last column of 'reduced' (after elimination).
double residualNorm(const Matrix &orig, const Matrix &reduced) {
    if (orig.n != reduced.n) return -1.0;
    int n = orig.n;
    std::vector<double> x(n);
    for (int i = 0; i < n; ++i) x[i] = reduced.data[i][n]; // last column

    double sumsq = 0.0;
    for (int i = 0; i < n; ++i) {
        double s = 0.0;
        for (int j = 0; j < n; ++j) s += orig.data[i][j] * x[j];
        double r = s - orig.data[i][n];
        sumsq += r * r;
    }
    return std::sqrt(sumsq);
}
