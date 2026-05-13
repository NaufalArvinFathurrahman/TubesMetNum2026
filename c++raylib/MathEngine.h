// ==========================================
// KELAS MATEMATIKA (MathEngine.h)
// ==========================================
// Berkas ini mendeklarasikan fungsi-fungsi matematika (Metode Numerik)
// yang digunakan untuk mengoptimasi lalu lintas. Semua fungsi bersifat statis (static)
// agar bisa langsung dipanggil tanpa perlu membuat objek MathEngine.

#ifndef MATHENGINE_H
#define MATHENGINE_H

#include <vector>

class MathEngine {
public:
    // Melakukan Dekomposisi Matriks (Memecah Matriks A menjadi Lower dan Upper)
    static void lu_decomposition(const std::vector<std::vector<double>>& A, std::vector<std::vector<double>>& L, std::vector<std::vector<double>>& U);
    
    // Menyelesaikan persamaan linear (mencari Vektor X / Arus Kendaraan)
    static std::vector<double> solve_lu(const std::vector<std::vector<double>>& L, const std::vector<std::vector<double>>& U, const std::vector<double>& B);
    
    // Algoritma Newton-Raphson untuk mencari durasi siklus lampu optimum
    static double newton_raphson(double initial_guess, double a, double b);
};

#endif