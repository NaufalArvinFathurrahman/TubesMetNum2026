// ==========================================
// IMPLEMENTASI MATEMATIKA (MathEngine.cpp)
// ==========================================
// Berisi algoritma murni yang tidak bergantung pada grafis.
// Fungsi-fungsi di sini mewakili "Otak" simulasi.

#include "MathEngine.h"
#include <cmath>
#include <iostream>

// Fungsi untuk melakukan dekomposisi matriks LU
void MathEngine::lu_decomposition(const std::vector<std::vector<double>>& A, std::vector<std::vector<double>>& L, std::vector<std::vector<double>>& U) {
    int n = A.size();
    
    // Inisialisasi Matriks L (Lower) dan U (Upper) dengan angka 0
    L.assign(n, std::vector<double>(n, 0.0));
    U.assign(n, std::vector<double>(n, 0.0));
    
    for (int i = 0; i < n; i++) {
        // --- Segitiga Atas (Upper Triangular) ---
        for (int k = i; k < n; k++) {
            double sum_upper = 0;
            for (int j = 0; j < i; j++) sum_upper += L[i][j] * U[j][k];
            U[i][k] = A[i][k] - sum_upper;
        }
        
        // --- Segitiga Bawah (Lower Triangular) ---
        for (int k = i; k < n; k++) {
            if (i == k) {
                // Diagonal utama dari matriks L selalu bernilai 1
                L[i][i] = 1.0;
            } else {
                double sum_lower = 0;
                for (int j = 0; j < i; j++) sum_lower += L[k][j] * U[j][i];
                L[k][i] = (A[k][i] - sum_lower) / U[i][i];
            }
        }
    }
}

// Fungsi substitusi Maju-Mundur untuk menyelesaikan persamaan Arus Kendaraan
std::vector<double> MathEngine::solve_lu(const std::vector<std::vector<double>>& L, const std::vector<std::vector<double>>& U, const std::vector<double>& B) {
    int n = L.size();
    std::vector<double> Y(n, 0.0);
    std::vector<double> X(n, 0.0); // X adalah vektor hasil (Arus riil di jalan)
    
    // Substitusi Maju (Forward Substitution): Mencari matriks penengah Y
    for (int i = 0; i < n; i++) {
        double sum_y = 0;
        for (int j = 0; j < i; j++) sum_y += L[i][j] * Y[j];
        Y[i] = B[i] - sum_y;
    }
    
    // Substitusi Mundur (Backward Substitution): Mencari hasil akhir X
    for (int i = n - 1; i >= 0; i--) {
        double sum_x = 0;
        for (int j = i + 1; j < n; j++) sum_x += U[i][j] * X[j];
        X[i] = (Y[i] - sum_x) / U[i][i];
    }
    
    return X;
}

// Fungsi Pencarian Akar (Titik Minimum Persamaan Tundaan Lalu Lintas)
double MathEngine::newton_raphson(double initial_guess, double a, double b) {
    double C_current = initial_guess; // C = Cycle Time (Durasi Siklus Lampu)
    
    // Maksimal melakukan iterasi sebanyak 100 kali
    for (int i = 0; i < 100; i++) {
        // Menghitung Nilai Fungsi f(C)
        double g_val = a - (b / std::pow(C_current, 2));
        
        // Menghitung Nilai Turunan Fungsi f'(C)
        double g_prime_val = 2 * b / std::pow(C_current, 3);
        
        // Pencegahan error jika turunan sangat kecil (menghindari pembagian dengan nol)
        if (std::abs(g_prime_val) < 1e-10) break;
        
        // Rumus inti Newton-Raphson: x_baru = x_lama - (f(x) / f'(x))
        double C_next = C_current - (g_val / g_prime_val);
        
        // Cek syarat berhenti: Jika perbedaannya sangat kecil, berarti sudah konvergen
        if (std::abs(C_next - C_current) < 1e-6) return C_next;
        
        // Update tebakan untuk iterasi selanjutnya
        C_current = C_next;
    }
    return C_current;
}