#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include "raylib.h"
#include "raymath.h"

using namespace std;

// ==========================================
// 1. MESIN MATEMATIKA: DEKOMPOSISI LU (Murni std::vector)
// ==========================================
void lu_decomposition(const vector<vector<double>>& A, vector<vector<double>>& L, vector<vector<double>>& U) {
    int n = A.size();
    L.assign(n, vector<double>(n, 0.0));
    U.assign(n, vector<double>(n, 0.0));

    for (int i = 0; i < n; i++) {
        for (int k = i; k < n; k++) {
            double sum_upper = 0;
            for (int j = 0; j < i; j++) sum_upper += L[i][j] * U[j][k];
            U[i][k] = A[i][k] - sum_upper;
        }
        for (int k = i; k < n; k++) {
            if (i == k) {
                L[i][i] = 1.0;
            } else {
                double sum_lower = 0;
                for (int j = 0; j < i; j++) sum_lower += L[k][j] * U[j][i];
                L[k][i] = (A[k][i] - sum_lower) / U[i][i];
            }
        }
    }
}

vector<double> solve_lu(const vector<vector<double>>& L, const vector<vector<double>>& U, const vector<double>& B) {
    int n = L.size();
    vector<double> Y(n, 0.0);
    vector<double> X(n, 0.0);

    for (int i = 0; i < n; i++) {
        double sum_y = 0;
        for (int j = 0; j < i; j++) sum_y += L[i][j] * Y[j];
        Y[i] = B[i] - sum_y;
    }
    for (int i = n - 1; i >= 0; i--) {
        double sum_x = 0;
        for (int j = i + 1; j < n; j++) sum_x += U[i][j] * X[j];
        X[i] = (Y[i] - sum_x) / U[i][i];
    }
    return X;
}

// ==========================================
// 2. PENCARIAN AKAR: NEWTON-RAPHSON
// ==========================================
double g(double C, double a, double b) { return a - (b / pow(C, 2)); }
double g_prime(double C, double b) { return 2 * b / pow(C, 3); }

double newton_raphson(double initial_guess, double a, double b, double tol = 1e-6, int max_iter = 100) {
    double C_current = initial_guess;
    cout << "\n--- Iterasi Newton-Raphson ---\n";
    for (int i = 0; i < max_iter; i++) {
        double g_val = g(C_current, a, b);
        double g_prime_val = g_prime(C_current, b);
        if (abs(g_prime_val) < 1e-10) break;
        
        double C_next = C_current - (g_val / g_prime_val);
        cout << "Iterasi " << i+1 << ": C = " << C_current << " detik\n";
        
        if (abs(C_next - C_current) < tol) return C_next;
        C_current = C_next;
    }
    return C_current;
}

// ==========================================
// 3. STRUKTUR AI KENDARAAN (RAYLIB 3D)
// ==========================================
struct Kendaraan {
    Vector3 posisi;
    Vector3 arah;
    float kecepatan;
    float max_kecepatan;
    string jalur; // "NS" atau "EW"
    Color warna;
};

// ==========================================
// FUNGSI UTAMA (MAIN)
// ==========================================
int main() {
    // --- FASE 1: KALKULASI MATEMATIKA ---
    vector<vector<double>> Matriks_A = {
        { 4, -1,  0, -1}, {-1,  4, -1,  0}, { 0, -1,  4, -1}, {-1,  0, -1,  3}
    };
    vector<double> Vektor_B = {120, 90, 150, 110};

    vector<vector<double>> L, U;
    lu_decomposition(Matriks_A, L, U);
    vector<double> flow_mentah = solve_lu(L, U, Vektor_B);

    double sum_flow = 0;
    for (double f : flow_mentah) sum_flow += f;
    
    double param_a = sum_flow * 0.05;
    double param_b = sum_flow * 150.0;
    double siklus_matematis = newton_raphson(60.0, param_a, param_b);
    
    // Interval visual kendaraan (agar tidak terlalu padat di layar)
    double interval_ns = (60.0 / ((flow_mentah[0] + flow_mentah[2]) / 2)) * 3;
    double interval_ew = (60.0 / ((flow_mentah[1] + flow_mentah[3]) / 2)) * 3;
    double siklus_visual = fmax(10.0, siklus_matematis / 5.0);

    // --- FASE 2: INISIALISASI RAYLIB ---
    const int screenWidth = 1000;
    const int screenHeight = 800;
    InitWindow(screenWidth, screenHeight, "Simulasi Lalu Lintas - Optimasi Numerik");

    Camera camera = { 0 };
    camera.position = (Vector3){ 25.0f, 30.0f, -25.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // State Simulasi
    vector<Kendaraan> daftar_mobil;
    bool hijau_ns = true;
    float timer_lampu = 0.0f;
    float timer_spawn_ns = 0.0f;
    float timer_spawn_ew = 0.0f;

    SetTargetFPS(60);

    // --- GAME LOOP UTAMA ---
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        // 1. Update Lampu Lalu Lintas
        timer_lampu += dt;
        if (timer_lampu > (siklus_visual / 2.0)) {
            timer_lampu = 0.0f;
            hijau_ns = !hijau_ns; // Tukar warna lampu
        }

        // 2. Spawner Mobil berdasarkan Flow Matriks
        timer_spawn_ns += dt;
        if (timer_spawn_ns > interval_ns) {
            timer_spawn_ns = 0.0f;
            int arah_z = (rand() % 2 == 0) ? 1 : -1;
            Kendaraan m = { 
                { 2.0f * arah_z, 0.5f, -30.0f * arah_z }, 
                { 0.0f, 0.0f, 1.0f * arah_z }, 
                15.0f, 15.0f, "NS", 
                (Color){ (unsigned char)(rand()%255), (unsigned char)(rand()%255), (unsigned char)(rand()%255), 255 }
            };
            daftar_mobil.push_back(m);
        }

        timer_spawn_ew += dt;
        if (timer_spawn_ew > interval_ew) {
            timer_spawn_ew = 0.0f;
            int arah_x = (rand() % 2 == 0) ? 1 : -1;
            Kendaraan m = { 
                { -30.0f * arah_x, 0.5f, -2.0f * arah_x }, 
                { 1.0f * arah_x, 0.0f, 0.0f }, 
                15.0f, 15.0f, "EW", 
                (Color){ (unsigned char)(rand()%255), (unsigned char)(rand()%255), (unsigned char)(rand()%255), 255 }
            };
            daftar_mobil.push_back(m);
        }

        // 3. Update Kesadaran AI Kendaraan
        for (auto& m : daftar_mobil) {
            bool harus_berhenti = false;

            // Logika Lampu Merah
            if (m.jalur == "NS" && !hijau_ns) {
                if (abs(m.posisi.z) < 8.0f && abs(m.posisi.z) > 5.0f) {
                    if ((m.posisi.z > 0 && m.arah.z < 0) || (m.posisi.z < 0 && m.arah.z > 0)) harus_berhenti = true;
                }
            } else if (m.jalur == "EW" && hijau_ns) { // Jika NS hijau, EW pasti merah
                if (abs(m.posisi.x) < 8.0f && abs(m.posisi.x) > 5.0f) {
                    if ((m.posisi.x > 0 && m.arah.x < 0) || (m.posisi.x < 0 && m.arah.x > 0)) harus_berhenti = true;
                }
            }

            // Logika Tabrakan Sederhana (Jarak dengan mobil di depan)
            for (auto& depan : daftar_mobil) {
                if (&m != &depan && m.jalur == depan.jalur && m.arah.x == depan.arah.x && m.arah.z == depan.arah.z) {
                    float jarak = Vector3Distance(m.posisi, depan.posisi);
                    // Cek apakah mobil lain berada di depannya (menggunakan dot product sederhana via posisi)
                    if (jarak < 4.0f) {
                        if (m.jalur == "NS" && m.posisi.z * m.arah.z < depan.posisi.z * depan.arah.z) harus_berhenti = true;
                        if (m.jalur == "EW" && m.posisi.x * m.arah.x < depan.posisi.x * depan.arah.x) harus_berhenti = true;
                    }
                }
            }

            // Pengereman atau Gas
            if (harus_berhenti) {
                m.kecepatan -= 30.0f * dt;
                if (m.kecepatan < 0) m.kecepatan = 0;
            } else {
                m.kecepatan += 10.0f * dt;
                if (m.kecepatan > m.max_kecepatan) m.kecepatan = m.max_kecepatan;
            }

            // Pindah posisi
            m.posisi.x += m.arah.x * m.kecepatan * dt;
            m.posisi.z += m.arah.z * m.kecepatan * dt;
        }

        // Hapus mobil yang sudah jauh
        for (auto it = daftar_mobil.begin(); it != daftar_mobil.end(); ) {
            if (abs(it->posisi.x) > 40.0f || abs(it->posisi.z) > 40.0f) it = daftar_mobil.erase(it);
            else ++it;
        }

        // --- FASE 3: RENDERING 3D ---
        BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode3D(camera);

        // Gambar Aspal Jalan
        DrawCube((Vector3){0, 0, 0}, 8.0f, 0.1f, 60.0f, DARKGRAY);
        DrawCube((Vector3){0, 0, 0}, 60.0f, 0.1f, 8.0f, DARKGRAY);

        // Gambar Indikator Lampu
        DrawSphere((Vector3){-5, 5, 5}, 1.0f, hijau_ns ? GREEN : RED); // Lampu NS
        DrawSphere((Vector3){5, 5, -5}, 1.0f, hijau_ns ? RED : GREEN); // Lampu EW

        // Gambar Semua Mobil
        for (const auto& m : daftar_mobil) {
            Vector3 ukuran = (m.jalur == "NS") ? (Vector3){2.0f, 1.5f, 4.0f} : (Vector3){4.0f, 1.5f, 2.0f};
            DrawCube(m.posisi, ukuran.x, ukuran.y, ukuran.z, m.warna);
            DrawCubeWires(m.posisi, ukuran.x, ukuran.y, ukuran.z, BLACK); // Garis tepi mobil
        }

        EndMode3D();
        
        // UI Teks di layar
        DrawText(TextFormat("Metode Numerik: Siklus Optimum (Newton-Raphson) = %.2f dtk", siklus_matematis), 10, 10, 20, BLACK);
        DrawText(TextFormat("Total Kendaraan Aktif: %d", daftar_mobil.size()), 10, 40, 20, DARKBLUE);
        
        EndDrawing();
    }

    CloseWindow();
    return 0;
}