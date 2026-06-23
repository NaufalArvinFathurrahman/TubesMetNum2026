#include <iostream>
#include <cmath>
#include <iomanip>

using namespace std;

// =================================================================
// MODUL 1: FUNGSI KALKULUS UNTUK NEWTON-RAPHSON
// =================================================================

// Ini adalah f'(C) [Turunan Pertama Delay Webster yang mau dijadikan 0]
// Kita menggunakan model representasi matematis: f'(C) = C^2 - (10000 * q_total) / C
double turunan_pertama(double C, double q_total) {
    return (C * C) - ((10000 * q_total) / C);
}

// Ini adalah f''(C) [Turunan Kedua, sebagai "penggaris/pembagi" untuk melompat]
// Turunan dari fungsi di atas.
double turunan_kedua(double C, double q_total) {
    return (2 * C) + ((10000 * q_total) / (C * C));
}

// =================================================================
// MODUL 2: FUNGSI DEKOMPOSISI LU (TANPA NUMPY/LIBRARY)
// =================================================================
void luDecomposition(double A[4][4], double B[4], double q[4]) {
    int n = 4;
    double L[4][4] = {0}, U[4][4] = {0};

    // 1. Memecah Matriks A menjadi L (Lower) dan U (Upper)
    for (int i = 0; i < n; i++) {
        // Upper Triangular
        for (int k = i; k < n; k++) {
            double sum = 0;
            for (int j = 0; j < i; j++)
                sum += (L[i][j] * U[j][k]);
            U[i][k] = A[i][k] - sum;
        }
        // Lower Triangular
        for (int k = i; k < n; k++) {
            if (i == k)
                L[i][i] = 1; // Diagonal L selalu 1
            else {
                double sum = 0;
                for (int j = 0; j < i; j++)
                    sum += (L[k][j] * U[j][i]);
                L[k][i] = (A[k][i] - sum) / U[i][i];
            }
        }
    }

    // 2. Substitusi Maju (Mencari nilai y pada L * y = B)
    double y[4] = {0};
    for (int i = 0; i < n; i++) {
        double sum = 0;
        for (int j = 0; j < i; j++) {
            sum += L[i][j] * y[j];
        }
        y[i] = B[i] - sum;
    }

    // 3. Substitusi Mundur (Mencari nilai q pada U * q = y)
    for (int i = n - 1; i >= 0; i--) {
        double sum = 0;
        for (int j = i + 1; j < n; j++) {
            sum += U[i][j] * q[j];
        }
        q[i] = (y[i] - sum) / U[i][i];
    }
}

// =================================================================
// MAIN PROGRAM (JALUR WAKTU EKSEKUSI)
// =================================================================
int main() {
    cout << "=== TAHAP 1: DEKOMPOSISI LU (MENCARI FLOW RATE) ===" << endl;
    
    // Matriks A: Data sensor koefisien antrean persimpangan 4 arah
    double Matriks_A[4][4] = {
        { 2, -1,  0,  0},
        {-1,  2, -1,  0},
        { 0, -1,  2, -1},
        { 0,  0, -1,  2}
    };
    
    // Matriks B: Konstanta volume kendaraan masuk
    double Matriks_B[4] = {0.2, 0.4, 0.3, 0.1}; 
    
    // Array untuk menyimpan hasil Flow Rate (q)
    double q_flow[4] = {0}; 

    // Eksekusi Algoritma LU
    luDecomposition(Matriks_A, Matriks_B, q_flow);

    // Ambil total flow rate dari seluruh persimpangan
    double q_total = 0;
    for (int i = 0; i < 4; i++) {
        cout << "Flow Rate Jalur " << i+1 << " (q" << i+1 << ") : " << fixed << setprecision(3) << q_flow[i] << " mobil/detik\n";
        q_total += q_flow[i];
    }
    cout << "Total Agregat Flow Rate (q_total): " << q_total << " mobil/detik\n\n";

    // =============================================================
    cout << "=== TAHAP 2: NEWTON-RAPHSON PADA FUNGSI WEBSTER ===" << endl;
    
    double C_lama = 10.0; // Tebakan awal siklus: 10 detik
    double C_baru = 0;
    double toleransi = 0.001;
    int iterasi = 1;

    cout << "Target: Mencari nilai C saat f'(C) = 0\n";
    cout << "Tebakan awal (C) = " << C_lama << " detik\n";

    while (true) {
        // Hitung nilai fungsi dengan C saat ini
        double nilai_f_aksen = turunan_pertama(C_lama, q_total);
        double nilai_f_double_aksen = turunan_kedua(C_lama, q_total);

        // Rumus inti Newton-Raphson
        C_baru = C_lama - (nilai_f_aksen / nilai_f_double_aksen);

        cout << "Iterasi " << iterasi << " | Lompat ke C = " << C_baru << " detik\n";

        // Cek syarat sukses: Apakah tebakan sudah stabil (tidak berubah)?
        if (abs(C_baru - C_lama) < toleransi) {
            break; // Jika ya, hentikan program
        }

        C_lama = C_baru;
        iterasi++;
        
        // Safety lock menghindari infinite loop
        if(iterasi > 100) break; 
    }

    // =============================================================
    cout << "\n=== HASIL FINAL OPTIMASI ===" << endl;
    cout << "Durasi Lampu Hijau Optimal (C) : " << round(C_baru) << " detik." << endl;

    return 0;
}