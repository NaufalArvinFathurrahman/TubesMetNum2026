// ==========================================
// KELAS PENGATUR PERSIMPANGAN (Persimpangan.h)
// ==========================================
// Berkas ini mendeklarasikan apa saja yang dimiliki oleh "Dunia" (World) Simulasi.
// Ini termasuk daftar seluruh mobil, state lampu merah, siklus waktu matematis, dll.

#ifndef INTERSECTION_H
#define INTERSECTION_H

#include "Kendaraan.h"
#include "raylib.h"
#include <vector>

class Persimpangan {
private:
    std::vector<Kendaraan> daftarKendaraan; // Tempat penampungan seluruh mobil yang aktif di layar
    double optimalCycleTime;       // Waktu (detik) siklus total Newton-Raphson
    float timerLampu;              // Stop-watch (timer) untuk transisi lampu
    float timerSpawn;              // Stop-watch (timer) jeda antar keluarnya mobil baru
    int faseSaatIni;              // Arah jalan yang sedang mendapat giliran: 0=N, 1=E, 2=S, 3=W
    bool sedangFaseKuning;            // Penanda apakah sekarang sedang transisi Lampu Kuning
    double yellowDuration;         // Maksimum detik untuk menyalanya lampu kuning

public:
    // Konstruktor
    Persimpangan(double calculatedCycleTime);
    
    // Dipanggil setiap detik untuk menjalankan fisika dan algoritma AI
    void perbaruiDunia(float dt, double jedaUtaraSelatan, double jedaTimurBarat);
    
    // Fungsi khusus menggambar balok, aspal, tiang, dan mobil dalam bentuk 3D Perspektif
    void drawWorld3D() const;
    
    // Fungsi khusus menggambar UI Teks 2D (seperti Hitung Mundur) di atas dunia 3D
    void drawWorld2D(Camera camera) const;
    
    // Fungsi pembuat mobil-mobil baru yang muncul dari pinggir map
    void munculkanKendaraan(float dt, double jedaUtaraSelatan, double jedaTimurBarat);
    
    // --- Getter & Setter ---
    // Dipanggil saat jam sibuk/sepi berganti untuk merubah kecepatan siklus di tengah jalan
    void setOptimalCycleTime(double newCycle) { optimalCycleTime = newCycle; }
    
    int getCurrentPhase() const { return faseSaatIni; }
    bool getIsYellowPhase() const { return sedangFaseKuning; }
};

#endif