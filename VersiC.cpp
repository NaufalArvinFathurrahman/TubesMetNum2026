#include <iostream>
#include <armadillo> // Library MATLAB-style untuk C++
#include <cmath>

using namespace std;
using namespace arma; // Memungkinkan pemanggilan syntax ala MATLAB

// ==========================================
// 1. SISTEM PERSAMAAN LINIER: DEKOMPOSISI LU
// ==========================================

// Fungsi memecah matriks A menjadi matriks Lower (L) dan Upper (U) dari awal
void lu_decomposition(const mat& A, mat& L, mat& U) {
    int n = A.n_rows; // Syntax khas MATLAB
    L.zeros(n, n);    // Inisialisasi matriks nol ala MATLAB
    U.zeros(n, n);

    for (int i = 0; i < n; i++) {
        // Upper Triangular
        for (int k = i; k < n; k++) {
            double sum_upper = 0;
            for (int j = 0; j < i; j++) {
                sum_upper += L(i, j) * U(j, k); // Mengakses elemen menggunakan (i, j)
            }
            U(i, k) = A(i, k) - sum_upper;
        }

        // Lower Triangular
        for (int k = i; k < n; k++) {
            if (i == k) {
                L(i, i) = 1.0; // Diagonal L selalu 1
            } else {
                double sum_lower = 0;
                for (int j = 0; j < i; j++) {
                    sum_lower += L(k, j) * U(j, i);
                }
                L(k, i) = (A(k, i) - sum_lower) / U(i, i);
            }
        }
    }
}

// Fungsi substitusi maju-mundur untuk mencari X (Flow Aktual)
vec solve_lu(const mat& L, const mat& U, const vec& B) {
    int n = L.n_rows;
    vec Y = zeros<vec>(n); // Vektor kolom nol
    vec X = zeros<vec>(n);

    // Substitusi Maju: L * Y = B
    for (int i = 0; i < n; i++) {
        double sum_y = 0;
        for (int j = 0; j < i; j++) {
            sum_y += L(i, j) * Y(j);
        }
        Y(i) = B(i) - sum_y;
    }

    // Substitusi Mundur: U * X = Y
    for (int i = n - 1; i >= 0; i--) {
        double sum_x = 0;
        for (int j = i + 1; j < n; j++) {
            sum_x += U(i, j) * X(j);
        }
        X(i) = (Y(i) - sum_x) / U(i, i);
    }

    return X;
}

// ==========================================
// 2. PENCARIAN AKAR: NEWTON-RAPHSON
// ==========================================

// Turunan pertama agregat delay (f'(C))
double g(double C, double a, double b) {
    return a - (b / pow(C, 2));
}

// Turunan kedua agregat delay (f''(C))
double g_prime(double C, double b) {
    return 2 * b / pow(C, 3);
}

// Algoritma Newton-Raphson dari awal
double newton_raphson_optimal_cycle(double initial_guess, double a, double b, double tolerance = 1e-6, int max_iter = 100) {
    double C_current = initial_guess;
    cout << "\n--- Memulai Iterasi Newton-Raphson untuk Siklus Optimum ---\n";
    
    for (int i = 0; i < max_iter; i++) {
        double g_val = g(C_current, a, b);
        double g_prime_val = g_prime(C_current, b);
        
        if (abs(g_prime_val) < 1e-10) {
            cout << "Turunan terlalu kecil, berhenti untuk menghindari pembagian dengan nol.\n";
            break;
        }
        
        double C_next = C_current - (g_val / g_prime_val);
        
        cout << "Iterasi " << i+1 << ": C = " << C_current << " detik | f'(C) = " << g_val << "\n";
        
        if (abs(C_next - C_current) < tolerance) {
            cout << "Konvergensi tercapai pada iterasi " << i+1 << "!\n";
            return C_next;
        }
        
        C_current = C_next;
    }
    
    return C_current;
}

// ==========================================
// EKSEKUSI PROGRAM UTAMA
// ==========================================
int main() {
    // Definisi matriks A dan Vektor B ala MATLAB
    mat Matriks_A = {
        { 4, -1,  0, -1},
        {-1,  4, -1,  0},
        { 0, -1,  4, -1},
        {-1,  0, -1,  3}
    };
    
    vec Vektor_B = {120, 90, 150, 110};

    cout << "--- Menyelesaikan Sistem Persamaan Arus (LU Decomposition) ---\n";
    
    mat L, U;
    lu_decomposition(Matriks_A, L, U);
    vec flow_aktual = solve_lu(L, U, Vektor_B);
    
    // Tampilkan hasil (Armadillo sudah menyediakan operator print bawaan)
    cout << "Vektor Flow Aktual (Kendaraan/Menit):\n";
    flow_aktual.print();

    // Persiapan Newton-Raphson
    double sum_flow = sum(flow_aktual); // Fungsi sum() bawaan Armadillo/MATLAB
    double param_a = sum_flow * 0.05;
    double param_b = sum_flow * 150.0;
    double tebakan_awal = 60.0;

    double siklus_optimum = newton_raphson_optimal_cycle(tebakan_awal, param_a, param_b);
    
    cout << "\n>> HASIL AKHIR: Durasi Siklus Total Optimum = " << siklus_optimum << " detik\n";

    return 0;
}