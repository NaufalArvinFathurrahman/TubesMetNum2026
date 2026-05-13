// ==========================================
// KELAS KENDARAAN (Vehicle.h)
// ==========================================
// Berkas ini mendeklarasikan struktur dan identitas dari setiap objek mobil.

#ifndef VEHICLE_H
#define VEHICLE_H

#include "raylib.h"
#include <string>
#include <vector>

class Vehicle {
private:
    Vector3 position;      // Posisi mobil di koordinat 3D (x, y, z)
    Vector3 direction;     // Vektor arah maju mobil (contoh: maju ke utara = {0, 0, -1})
    float maxSpeed;        // Kecepatan maksimal mesin mobil
    float currentSpeed;    // Kecepatan aktual saat ini (bisa menurun saat mengerem)
    std::string origin;    // Asal arah mobil ("N", "S", "E", "W")
    Color color;           // Warna cat mobil (random)
    
    int turnChoice;        // Keputusan berbelok: 0 = Lurus, 1 = Kiri, 2 = Kanan
    bool hasTurned;        // Status penanda apakah mobil sudah selesai membelok di tengah persimpangan

public:
    // Konstruktor: Dipanggil saat mobil baru dilahirkan (spawn)
    Vehicle(Vector3 startPos, Vector3 startDir, std::string startOrigin, Color vehicleColor);
    
    // Fungsi untuk memikirkan logika pergerakan mobil di setiap detiknya
    void updateBehavior(float dt, int currentPhase, const std::vector<Vehicle>& otherVehicles, bool isYellowPhase);
    
    // Fungsi untuk menggambar bentuk 3D mobil ke layar
    void draw() const;
    
    // --- Getter Functions ---
    // Fungsi kecil untuk mengambil informasi privat dari luar kelas
    Vector3 getPosition() const { return position; }
    std::string getOrigin() const { return origin; }
    float getSpeed() const { return currentSpeed; }
};

#endif