#include "src_gauss_jordan.h"
#include <chrono>
#include <iostream>
#include <string>
#include <algorithm>
#include <thread>
#include <iomanip>  // at the top of your file



static int parseIntOrDefault(const char *s, int def) {
    try {
        return std::stoi(s);
    } catch (...) {
        return def;
    }
}

int main(int argc, char **argv) {
    // default parameters
    int size = 256;         // default matrix dimension (n)
    unsigned threadCount = 0; // 0 -> auto detect hardware_concurrency
    bool runParallel = true;

    // simple CLI:
    // --size N
    // --threads N   (0 = auto)
    // --seq          run sequential version only
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--size" && i + 1 < argc) {
            size = parseIntOrDefault(argv[++i], size);
        } else if (a == "--threads" && i + 1 < argc) {
            threadCount = static_cast<unsigned>(parseIntOrDefault(argv[++i], 0));
        } else if (a == "--seq") {
            runParallel = false;
        } else if (a == "--help") {
            std::cout << "Usage: " << argv[0] << " [--size N] [--threads N] [--seq]\n";
            return 0;
        }
    }

    std::cout << "Gauss-Jordan benchmark (n = " << size << ")\n";

    Matrix orig(size);
    orig.fillRandom();

    // Work on copies because the algorithm modifies the matrix in-place
    Matrix Aseq = orig;
    Matrix Apar = orig;

    // Sequential run
    std::cout << "Running sequential Gauss-Jordan...\n";
    auto t0 = std::chrono::high_resolution_clock::now();
    try {
        gaussJordanSequential(Aseq);
    } catch (const std::exception &ex) {
        std::cerr << "Error in sequential: " << ex.what() << "\n";
        return 1;
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    double ms_seq = std::chrono::duration<double, std::milli>(t1 - t0).count();
    double res_seq = residualNorm(orig, Aseq);
    std::cout << std::fixed << std::setprecision(20);
    std::cout << "Sequential time: " << ms_seq << " ms, residual ||Ax-b|| = " << res_seq << "\n";

    if (runParallel) {
        if (threadCount == 0) {
            unsigned hw = std::thread::hardware_concurrency();
            threadCount = hw > 0 ? hw : 1;
        }
        std::cout << "Running parallel Gauss-Jordan with " << threadCount << " threads...\n";

        auto t2 = std::chrono::high_resolution_clock::now();
        try {
            gaussJordanParallel(Apar, threadCount);
        } catch (const std::exception &ex) {
            std::cerr << "Error in parallel: " << ex.what() << "\n";
            return 1;
        }
        auto t3 = std::chrono::high_resolution_clock::now();
        double ms_par = std::chrono::duration<double, std::milli>(t3 - t2).count();
        double res_par = residualNorm(orig, Apar);
        std::cout << std::fixed << std::setprecision(20);
        std::cout << "Parallel time: " << ms_par << " ms, residual ||Ax-b|| = " << res_par << "\n";

    }

    return 0;
}
