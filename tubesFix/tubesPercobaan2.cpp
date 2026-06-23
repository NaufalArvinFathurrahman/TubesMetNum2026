#include <iostream>
#include <iomanip>

using namespace std;

// =================================================================
// FUNGSI UTILITAS MATEMATIKA DASAR (PENGGANTI <cmath>)
// =================================================================

// Fungsi untuk mencari nilai mutlak (absolut)
double nilai_mutlak(double x) {
    if (x < 0) {
        return -x;
    }
    return x;
}

// =================================================================
// TAHAP 1: MODUL DEKOMPOSISI LU (Sistem Persamaan Linier)
// =================================================================

void faktorisasi_LU_dan_substitusi(double A[4][4], double B[4], double q[4]) {
    double L[4][4] = {0};
    double U[4][4] = {0};
    double y[4] = {0};

    // 1. Proses Dekomposisi A menjadi L dan U
    for (int i = 0; i < 4; i++) {
        // Mencari nilai untuk matriks Upper (U)
        for (int k = i; k < 4; k++) {
            double sum = 0;
            for (int j = 0; j < i; j++) {
                sum += (L[i][j] * U[j][k]);
            }
            U[i][k] = A[i][k] - sum;
        }

        // Mencari nilai untuk matriks Lower (L)
        for (int k = i; k < 4; k++) {
            if (i == k) {
                L[i][i] = 1; // Diagonal utama L selalu 1
            } else {
                double sum = 0;
                for (int j = 0; j < i; j++) {
                    sum += (L[k][j] * U[j][i]);
                }
                L[k][i] = (A[k][i] - sum) / U[i][i];
            }
        }
    }

    // 2. Forward Substitution (L * y = B)
    for (int i = 0; i < 4; i++) {
        double sum = 0;
        for (int j = 0; j < i; j++) {
            sum += L[i][j] * y[j];
        }
        y[i] = B[i] - sum;
    }

    // 3. Backward Substitution (U * q = y)
    for (int i = 3; i >= 0; i--) {
        double sum = 0;
        for (int j = i + 1; j < 4; j++) {
            sum += U[i][j] * q[j];
        }
        q[i] = (y[i] - sum) / U[i][i];
    }
}

// =================================================================
// TAHAP 2: MODUL FUNGSI TUNDAAN WEBSTER
// =================================================================

// Fungsi untuk menghitung total kemacetan (d)
double hitung_delay_webster(double C, double* q, double s, double L) {
    double delay_total = 0;
    double Y = 0;
    
    // Menghitung rasio arus (Y) untuk 4 fase
    for (int i = 0; i < 4; i++) {
        Y += q[i] / s;
    }
    
    // Mencegah C lebih kecil dari L
    if (C <= L) return 999999.0;

    for (int i = 0; i < 4; i++) {
        double y_i = q[i] / s;
        // Pembagian waktu hijau efektif secara proporsional
        double g_i = (y_i / Y) * (C - L);
        
        double lambda = g_i / C;            
        double x = q[i] / (lambda * s);      
        
        // Mencegah kejenuhan lebih dari 1 (kapasitas terlampaui)
        if (x >= 1.0) return 999999.0;

        // Menghitung Suku Pertama (Seragam)
        double pengali_1 = (1 - lambda);
        double pembilang_d1 = C * (pengali_1 * pengali_1); 
        double penyebut_d1 = 2 * (1 - (lambda * x));
        double d1 = pembilang_d1 / penyebut_d1;

        // Menghitung Suku Kedua (Acak)
        double pembilang_d2 = x * x; 
        double penyebut_d2 = 2 * q[i] * (1 - x);
        double d2 = pembilang_d2 / penyebut_d2;

        delay_total += (d1 + d2);
    }

    return delay_total;
}

// Turunan Pertama Numerik: f'(C)
double turunan_pertama(double C, double* q, double s, double L) {
    double h = 0.001; 
    double delay_maju = hitung_delay_webster(C + h, q, s, L);
    double delay_sekarang = hitung_delay_webster(C, q, s, L);
    
    return (delay_maju - delay_sekarang) / h; 
}

// Turunan Kedua Numerik: f''(C)
double turunan_kedua(double C, double* q, double s, double L) {
    double h = 0.001;
    double f_aksen_maju = turunan_pertama(C + h, q, s, L);
    double f_aksen_sekarang = turunan_pertama(C, q, s, L);
    
    return (f_aksen_maju - f_aksen_sekarang) / h;
}

// =================================================================
// TAHAP 3: EKSEKUSI PROGRAM UTAMA
// =================================================================

int main() {
    // --- SETUP MATRIKS PERSIMPANGAN ---
    // Matriks koefisien hambatan antar arah (A) dan kapasitas net (B)
    double A[4][4] = {
        { 2, -1,  0,  0},
        {-1,  2, -1,  0},
        { 0, -1,  2, -1},
        { 0,  0, -1,  2}
    };
    double B[4] = {0.20, 0.40, 0.30, 0.10};
    double q[4] = {0}; // Wadah untuk menyimpan hasil Flow Rate
    
    cout << "=== TAHAP 1: HASIL DEKOMPOSISI LU ===" << endl;
    faktorisasi_LU_dan_substitusi(A, B, q);
    
    double q_total = 0;
    for (int i = 0; i < 4; i++) {
        cout << "Flow Rate Jalur " << i + 1 << " (q" << i + 1 << "): " 
             << fixed << setprecision(3) << q[i] << " mobil/detik" << endl;
        q_total += q[i];
    }
    cout << "-----------------------------------" << endl;
    cout << "Total Agregat (q_total) : " << q_total << " mobil/detik\n\n";

    // --- SETUP NEWTON-RAPHSON ---
    cout << "=== TAHAP 2: OPTIMASI NEWTON-RAPHSON PADA WEBSTER ===" << endl;
    
    // Parameter Lapangan
    double arus_jenuh_s = 20.0; // Saturation flow gabungan (kapasitas maksimal aspal)
    double waktu_hilang_L = 15.0; // Total Waktu Hilang / Lost Time (L)
    
    // Parameter Mesin Iterasi
    double C_sekarang = 20.0; // Tebakan awal waktu siklus (diubah agar tidak menjauh)
    double C_berikutnya = 0;
    double toleransi = 0.001;
    int maks_iterasi = 100;    // Batas aman infinite loop
    int iterasi = 1;

    while (iterasi <= maks_iterasi) {
        double f_aksen = turunan_pertama(C_sekarang, q, arus_jenuh_s, waktu_hilang_L);
        double f_double_aksen = turunan_kedua(C_sekarang, q, arus_jenuh_s, waktu_hilang_L);
        
        // Mencegah error pembagian dengan nol
        if (nilai_mutlak(f_double_aksen) < 0.000001) {
            cout << "Peringatan: Turunan kedua mendekati nol, iterasi dihentikan paksa." << endl;
            break;
        }

        // Lompatan Newton-Raphson
        C_berikutnya = C_sekarang - (f_aksen / f_double_aksen);
        
        cout << "Iterasi " << iterasi << " | C bergeser ke: " 
             << fixed << setprecision(3) << C_berikutnya << " detik" << endl;
        
        // Cek syarat keberhasilan f'(C) = 0 (perubahan sangat kecil)
        if (nilai_mutlak(C_berikutnya - C_sekarang) < toleransi) {
            break; 
        }
        
        C_sekarang = C_berikutnya;
        iterasi++;
    }

    if (iterasi > maks_iterasi) {
        cout << "\n[!] Gagal: Iterasi mencapai batas maksimal tanpa konvergensi." << endl;
    }

    cout << "\n=== HASIL FINAL OPTIMASI ===" << endl;
    cout << "Durasi Siklus Lampu Lalu Lintas Optimal (C): " 
         << fixed << setprecision(0) << C_berikutnya << " detik." << endl;
         
    return 0;
}