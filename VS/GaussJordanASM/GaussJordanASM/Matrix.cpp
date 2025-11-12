// Plik Matrix.cpp - fragment do implementacji
#include "Matrix.h"
#include <iostream>
#include <stdexcept>
#include <cmath>
#include <random>
#include <iomanip>
#include <thread>

Matrix::Matrix(int n) : size(n), columns(n + 1)
{
    if (n <= 0) {
        throw std::invalid_argument("Rozmiar macierzy musi byc wiekszy niz 0.");
    }
    // Alokacja pamięci dla spłaszczonej macierzy (N * (N+1) elementów typu double)
    data.resize(size * columns);
}

void Matrix::generate_random()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> distrib(1.0, 10.0); 

    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < columns; ++j) {
            at(i, j) = distrib(gen);
        }
    }
}

void Matrix::print() const
{
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < columns; ++j) {
            std::cout << std::fixed << std::setprecision(2) << at(i, j) << "\t";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}


// Funkcja wykonująca kluczową operację na wierszach 
void Matrix::subtract_row_single_thread(int dest_row, int src_row, double factor)
{
    // Odwzorowuje operację: W_dest <- W_dest - factor * W_src
    int offset = dest_row * columns;
    int src_offset = src_row * columns;

    // Pętla C++ (punkt do porównania z SIMD ASM)
    for (int j = 0; j < columns; ++j)
    {
        data[offset + j] -= factor * data[src_offset + j];
    }
}

// Główna funkcja eliminacji (część pętli eliminacji)
void Matrix::perform_elimination(int start_row, int end_row, int pivot_col, double factor, int implementation_id)
{
    // subtract_row_single_thread(start_row, pivot_col, factor); 
    // W późniejszym etapie tu będzie logika wielowątkowa i wybór C++/ASM.
}


void Matrix::elimination_worker(int start_row, int end_row, int pivot_row, double factor_unused, ImplementationType impl)
{
    // factor_unused jest tu nieużywany, ponieważ współczynnik jest wyliczany wewnątrz
    // pętli na podstawie at(i, pivot_row) (po normalizacji).

    // Iterujemy tylko przez wiersze w naszym zakresie
    for (int i = start_row; i < end_row; ++i)
    {
        if (i != pivot_row) // Nie modyfikujemy wiersza wiodącego
        {
            // Współczynnik dla wiersza i
            double current_factor = at(i, pivot_row);

            // Wywołujemy krytyczną operację (obecnie bazowa wersja C++)
            subtract_row_single_thread(i, pivot_row, current_factor);

            // Ustawienie elementu na 0, aby macierz była w czystej postaci Gaussa-Jordana
            at(i, pivot_row) = 0.0;
        }
    }
}

double Matrix::eliminate(ImplementationType impl, int num_threads)
{
    // Rozpoczynamy pomiar czasu
    auto start = std::chrono::high_resolution_clock::now();

    for (int k = 0; k < size; ++k) // Pętla po kolumnach (pivot)
    {
        // 1. Znajdowanie elementu wiodącego (Pivot Selection)
        int pivot_row = k;
        double max_val = std::abs(at(k, k));

        for (int i = k + 1; i < size; ++i) {
            if (std::abs(at(i, k)) > max_val) {
                max_val = std::abs(at(i, k));
                pivot_row = i;
            }
        }

        // Sprawdzenie, czy macierz jest osobliwa (może spowodować dzielenie przez zero)
        if (max_val == 0.0) {
            // W praktyce należy obsłużyć jako błąd (np. brak unikalnego rozwiązania)
            // Na potrzeby projektu możemy na razie zignorować lub przerwać.
            // std::cerr << "Macierz osobliwa! Brak kontynuacji eliminacji.\n";
            // break;
        }

        // 2. Zamiana wierszy (Pivot Swap)
        if (pivot_row != k) {
            // Użycie std::swap do zamiany wierszy (całych fragmentów pamięci)
            // Wiersz k zaczyna się na data[k * columns], wiersz pivot_row na data[pivot_row * columns]
            // std::vector ma wygodną funkcję std::swap_ranges, ale wygodniej będzie użyć std::swap
            for (int j = 0; j < columns; ++j) {
                std::swap(at(k, j), at(pivot_row, j));
            }
        }

        double pivot_val = at(k, k);
        if (pivot_val == 0.0) continue; // Uniknięcie dzielenia przez zero po swapie

        // 3. Normalizacja wiersza wiodącego (Normalization)
        // Dzielenie całego wiersza k przez wartość at(k, k), aby element at(k, k) był równy 1.
        for (int j = 0; j < columns; ++j) {
            at(k, j) /= pivot_val;
        }

        // 4. Eliminacja pozostałych wierszy (Elimination)
        if (impl == CPP_STANDARD)
        {
            // Wersja jednowątkowa - Wykonujemy eliminację dla KAŻDEGO wiersza 'i'
            for (int i = 0; i < size; ++i)
            {
                if (i != k)
                {
                    double factor = at(i, k); // Po normalizacji, to jest współczynnik, przez który mnożymy W_k

                    // Wywołanie krytycznej operacji. 
                    subtract_row_single_thread(i, k, factor);

                    // Ustawienie elementu na 0
                    at(i, k) = 0.0;
                }
            }
        }
        else if (impl == CPP_MULTITHREAD)
        {
            // Wersja WIELOWĄTKOWA - Dzielimy CAŁĄ pracę eliminacji na wątki

            // Ograniczamy liczbę wątków do rozmiaru macierzy (size), ale też do max 64.
            num_threads = std::min({ num_threads, size, 64 });

            std::vector<std::thread> threads;

            // Obliczamy wielkość bloku wierszy dla każdego wątku
            int rows_to_process = size;
            int rows_per_thread = rows_to_process / num_threads;
            int start_row = 0;

            for (int t = 0; t < num_threads; ++t)
            {
                int end_row = start_row + rows_per_thread;

                // Ostatni wątek bierze resztę wierszy (jeśli podział jest nierówny)
                if (t == num_threads - 1) {
                    end_row = size;
                }

                // UWAGA: Nie musimy sprawdzać if (end_row > start_row), ponieważ 
                // num_threads jest już ograniczony do size.

                // Uruchomienie wątku roboczego
                // Wątek wykonuje eliminację dla swojego zakresu [start_row, end_row)
                threads.emplace_back(&Matrix::elimination_worker, this,
                    start_row, end_row, k, at(k, k), impl);

                start_row = end_row;
            }

            // Oczekiwanie na zakończenie wszystkich wątków (Krytyczne dla synchronizacji)
            for (auto& t : threads)
            {
                if (t.joinable()) {
                    t.join();
                }
            }
        }
    }

    // Zakończenie pomiaru czasu
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count(); // Zwraca czas w ms
}

