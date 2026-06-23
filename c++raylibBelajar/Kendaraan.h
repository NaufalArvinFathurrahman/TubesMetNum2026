// ==========================================
// KELAS KENDARAAN (Kendaraan.h)
// ==========================================
// Berkas ini mendeklarasikan struktur dan identitas dari setiap objek mobil.

#ifndef VEHICLE_H
#define VEHICLE_H

#include "raylib.h"
#include <string>
#include <vector>

class Kendaraan {
private:
    Vector3 posisi;      // Posisi mobil di koordinat 3D (x, y, z)
    Vector3 arah;     // Vektor arah maju mobil (contoh: maju ke utara = {0, 0, -1})
    float kecepatanMaks;        // Kecepatan maksimal mesin mobil
    float kecepatanSaatIni;    // Kecepatan aktual saat ini (bisa menurun saat mengerem)
    std::string asal;    // Asal arah mobil ("N", "S", "E", "W")
    Color warna;           // Warna cat mobil (random)
    
    int pilihanBelok;        // Keputusan berbelok: 0 = Lurus, 1 = Kiri, 2 = Kanan
    bool sudahBelok;        // Status penanda apakah mobil sudah selesai membelok di tengah persimpangan

public:
    // Konstruktor: Dipanggil saat mobil baru dilahirkan (spawn)
    Kendaraan(Vector3 startPos, Vector3 startDir, std::string startOrigin, Color vehicleColor);
    
    // Fungsi untuk memikirkan logika pergerakan mobil di setiap detiknya
    void perbaruiPerilaku(float dt, int faseSaatIni, const std::vector<Kendaraan>& otherVehicles, bool sedangFaseKuning);
    
    // Fungsi untuk menggambar bentuk 3D mobil ke layar
    void gambar() const;
    
    // --- Getter Functions ---
    // Fungsi kecil untuk mengambil informasi privat dari luar kelas
    Vector3 getPosition() const { return posisi; }
    std::string getOrigin() const { return asal; }
    float getSpeed() const { return kecepatanSaatIni; }
};

#endif