#include <iostream>
#include <vector>
#include <iomanip>
#include <chrono>
#include <thread>


using namespace std;

// 2D rozmycie Gaussa
vector<vector<double>> gaussianBlur2D(const vector<vector<double>>& input) {
    vector<vector<double>> kernel = {
            {0.06, 0.24, 0.40, 0.24, 0.06},
            {0.24, 0.40, 0.60, 0.40, 0.24},
            {0.40, 0.60, 1.00, 0.60, 0.40},
            {0.24, 0.40, 0.60, 0.40, 0.24},
            {0.06, 0.24, 0.40, 0.24, 0.06}
    };

    // Normalizacja kernela
    double sumKernel = 0.0;
    for (auto& row : kernel)
        for (auto v : row)
            sumKernel += v;
    for (auto& row : kernel)
        for (auto& v : row)
            v /= sumKernel;

    int rows = input.size();
    int cols = input[0].size();
    int half = kernel.size() / 2;

    vector<vector<double>> output(rows, vector<double>(cols, 0.0));

    // Dla każdego piksela
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            double sum = 0.0;

            // Dla każdego elementu filtra
            for (int ki = -half; ki <= half; ki++) {
                for (int kj = -half; kj <= half; kj++) {
                    int ni = i + ki;
                    int nj = j + kj;

                    // Sprawdzenie czy nie wychodzi poza obraz
                    if (ni >= 0 && ni < rows && nj >= 0 && nj < cols) {
                        sum += input[ni][nj] * kernel[ki + half][kj + half];
                    }
                }
            }

            output[i][j] = sum;
        }
    }

    return output;
}

vector<vector<double>> gaussianBlur2D_parallel(const vector<vector<double>>& image, int threadCount) {
    int height = image.size();
    int width = image[0].size();
    vector<vector<double>> result(height, vector<double>(width, 0.0));

    // kernel Gaussa (3x3)
    vector<vector<double>> kernel = {
            {1, 2, 1},
            {2, 4, 2},
            {1, 2, 1}
    };
    double sumKernel = 16.0;

    auto worker = [&](int startY, int endY) {
        for (int y = startY; y < endY; ++y) {
            for (int x = 0; x < width; ++x) {
                double sum = 0.0;
                for (int ky = -1; ky <= 1; ++ky) {
                    for (int kx = -1; kx <= 1; ++kx) {
                        int iy = min(max(y + ky, 0), height - 1);
                        int ix = min(max(x + kx, 0), width - 1);
                        sum += image[iy][ix] * kernel[ky + 1][kx + 1];
                    }
                }
                result[y][x] = sum / sumKernel;
            }
        }
    };

    // 🔹 Dzielimy obraz między wątki
    vector<thread> threads;
    int rowsPerThread = height / threadCount;
    int startY = 0;

    for (int i = 0; i < threadCount; ++i) {
        int endY = (i == threadCount - 1) ? height : startY + rowsPerThread;
        threads.emplace_back(worker, startY, endY);
        startY = endY;
    }

    // 🔹 Czekamy aż wszystkie wątki skończą
    for (auto& t : threads)
        t.join();

    return result;
}


int main() {
    vector<vector<double>> image = {
            {10, 20, 30, 40, 50},
            {20, 30, 40, 50, 60},
            {30, 40, 50, 60, 70},
            {40, 50, 60, 70, 80},
            {50, 60, 70, 80, 90}
    };

    int threadCount = 4; // np. 4 wątki – możemy później zmieniać

    cout << "Rozpoczynam rozmycie Gaussowskie (" << threadCount << " watki)...\n";
    auto start = chrono::high_resolution_clock::now();

    auto blurred = gaussianBlur2D_parallel(image, threadCount);

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> duration = end - start;

    cout << "Czas wykonania: " << duration.count() << " ms\n";

    cout << "Po rozmyciu:\n";
    for (auto& row : blurred) {
        for (auto v : row)
            cout << setw(8) << fixed << setprecision(1) << v;
        cout << "\n";
    }
}
