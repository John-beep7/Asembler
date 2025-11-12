#pragma once

#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <numeric>

class Matrix
{
private:
    std::vector<double> data; // Spłaszczona macierz 1D dla lepszej wydajności i ASM
    int size;                // Liczba wierszy (macierz N x N+1)
    int columns;             // Liczba kolumn (N + 1)

    // Funkcja wykonująca kluczową operację na wierszach (do zastąpienia przez ASM)
    void subtract_row_single_thread(int dest_row, int src_row, double factor);

    // Nowa wersja, która będzie mogła wywołać implementację C++ lub ASM
    void perform_elimination(int start_row, int end_row, int pivot_col, double factor, int implementation_id);

public:
    // 1 - C++ Standard; 2 - C++ Multi-Threaded; 3 - ASM SIMD
    enum ImplementationType { CPP_STANDARD, CPP_MULTITHREAD, ASM_SIMD };

    Matrix(int n);
    void generate_random();
    void print() const;

    // Główna funkcja wykonująca Eliminację Gaussa-Jordana
    double eliminate(ImplementationType impl, int num_threads = 1);

    // Metoda pomocnicza do pobrania liczby procesorów logicznych
    static int get_logical_processors() { return (int)std::thread::hardware_concurrency(); }

    // Dostęp do elementów (R, C)
    double& at(int r, int c) { return data[r * columns + c]; }
    const double& at(int r, int c) const { return data[r * columns + c]; }
};