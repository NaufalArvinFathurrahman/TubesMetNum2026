// ==========================================
// KELAS PENGATUR PERSIMPANGAN (Intersection.h)
// ==========================================
// Berkas ini mendeklarasikan apa saja yang dimiliki oleh "Dunia" (World) Simulasi.
// Ini termasuk daftar seluruh mobil, state lampu merah, siklus waktu matematis, dll.

#ifndef INTERSECTION_H
#define INTERSECTION_H

#include "Vehicle.h"
#include "raylib.h"
#include <vector>

class Intersection {
private:
    std::vector<Vehicle> vehicles; // Tempat penampungan seluruh mobil yang aktif di layar
    double optimalCycleTime;       // Waktu (detik) siklus total Newton-Raphson
    float timerLampu;              // Stop-watch (timer) untuk transisi lampu
    float timerSpawn;              // Stop-watch (timer) jeda antar keluarnya mobil baru
    int currentPhase;              // Arah jalan yang sedang mendapat giliran: 0=N, 1=E, 2=S, 3=W
    bool isYellowPhase;            // Penanda apakah sekarang sedang transisi Lampu Kuning
    double yellowDuration;         // Maksimum detik untuk menyalanya lampu kuning

public:
    // Konstruktor
    Intersection(double calculatedCycleTime);
    
    // Dipanggil setiap detik untuk menjalankan fisika dan algoritma AI
    void updateWorld(float dt, double intervalNS, double intervalEW);
    
    // Fungsi khusus menggambar balok, aspal, tiang, dan mobil dalam bentuk 3D Perspektif
    void drawWorld3D() const;
    
    // Fungsi khusus menggambar UI Teks 2D (seperti Hitung Mundur) di atas dunia 3D
    void drawWorld2D(Camera camera) const;
    
    // Fungsi pembuat mobil-mobil baru yang muncul dari pinggir map
    void spawnVehicles(float dt, double intervalNS, double intervalEW);
    
    // --- Getter & Setter ---
    // Dipanggil saat jam sibuk/sepi berganti untuk merubah kecepatan siklus di tengah jalan
    void setOptimalCycleTime(double newCycle) { optimalCycleTime = newCycle; }
    
    int getCurrentPhase() const { return currentPhase; }
    bool getIsYellowPhase() const { return isYellowPhase; }
};

#endif