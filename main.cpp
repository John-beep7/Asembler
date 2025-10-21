#include "src_gauss_jordan.h"
#include <chrono>
#include <iostream>

int main() {
    Matrix m(500); // you can start small (e.g., 10, 100, 500, 1000)
    m.fillRandom();

    std::cout << "Starting Gauss Jordan elimination..." << std::endl;

    auto start = std::chrono::high_resolution_clock::now();
    m.gaussJordanElimination();
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> duration = end - start;

    std::cout << "Elapsed time: " << duration.count() << " ms\n";
    std::cout << "Computation finished.\n";

    return 0;
}
