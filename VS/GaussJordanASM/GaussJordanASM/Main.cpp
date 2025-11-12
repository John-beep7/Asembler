#include <iostream>
#include <iomanip>
#include <cmath>
#include <windows.h>
#include "Matrix.h"

// Funkcja pomocnicza do wykonania pomiarów i uśredniania
void run_test(int n, Matrix::ImplementationType impl, int num_threads, const std::string& impl_name)
{
    const int NUM_CALLS = 5;
    double total_time = 0.0;

    for (int i = 0; i < NUM_CALLS + 1; ++i)
    {
        Matrix m(n);
        m.generate_random();

        if (i > 0)
        {
            double current_time = m.eliminate(impl, num_threads);
            total_time += current_time;

            // Debug: std::cout << "Wywołanie " << i << ": " << current_time << " ms\n";
        }
        else
        {
            m.eliminate(impl, num_threads);
        }
    }

    double average_time = total_time / NUM_CALLS;

    std::cout << "| " << n << "x" << n + 1 << " | " << std::setw(20) << impl_name
        << " | " << std::setw(8) << num_threads << " | "
        << std::fixed << std::setprecision(5) << average_time << " ms |\n";
}

int main()
{
    // Wykrycie domyślnej liczby wątków
    int max_threads = Matrix::get_logical_processors();
    std::cout << "number of logical proces (default): " << max_threads << std::endl;
    std::cout << "----------------------------------------------------------------------\n";

    // Zestawy danych S, M, L
    std::vector<int> sizes = { 64, 512, 1024 };
    // Konfiguracje wątków
    std::vector<int> thread_configs = { 1, 2, 4, 8, 16, 32, 64 };

    std::cout << "| Size    | " << std::setw(20) << "Implementation" << " | Threats | Avg. time (ms) |\n";
    std::cout << "|---------|----------------------|---------|---------------|\n";

    // Testowanie standardowej implementacji C++ (jednowątkowej)
    for (int size : sizes)
    {
        run_test(size, Matrix::CPP_STANDARD, 1, "C++ Standard");
    }

    // --- W tym miejscu będziesz dodawał testy C++ Multi-threaded i ASM SIMD ---

    std::cout << "----------------------------------------------------------------------\n";
    
    
    
    
    std::cout << "End of counting. Press ENTER, to shut down program.\n";
    std::cin.get(); // Czeka na wciśnięcie klawisza ENTER


    return 0;
}