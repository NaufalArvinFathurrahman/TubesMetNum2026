// ==========================================
// IMPLEMENTASI PENGATUR PERSIMPANGAN (Persimpangan.cpp)
// ==========================================
// Berkas ini berfungsi mengatur semua keadaan global di dalam simulasi,
// termasuk pergerakan lampu kuning/merah, spawn mobil baru,
// dan pemanggilan render 3D/2D untuk aspal, rambu, dan UI.

#include "Persimpangan.h"
#include "raymath.h"
#include <cstdlib>
#include <cmath>
#include <string>

// Konstruktor: Inisialisasi awal persimpangan
Persimpangan::Persimpangan(double calculatedCycleTime) {
    optimalCycleTime = calculatedCycleTime;
    timerLampu = 0.0f;
    timerSpawn = 0.0f;
    faseSaatIni = 0; // Mulai dengan arah Utara (N) = Hijau
    sedangFaseKuning = false;
    yellowDuration = 3.0; // Ini batas default awal, aslinya diatur dinamis di dalam perbaruiDunia
}

// Fungsi Penciptaan Mobil (Spawner)
void Persimpangan::munculkanKendaraan(float dt, double jedaUtaraSelatan, double jedaTimurBarat) {
    timerSpawn += dt;
    
    // Gunakan interval dinamis berdasarkan jam sibuk/sepi (rata-rata kecepatan spawn)
    double avgInterval = (jedaUtaraSelatan + jedaTimurBarat) / 2.0;

    // Jika timer sudah melebihi interval, maka tiba saatnya mengeluarkan mobil baru
    if (timerSpawn > avgInterval) {
        timerSpawn = 0.0f;
        
        // Buat warna acak (Random Color) bernuansa cerah
        Color c = { (unsigned char)(rand()%200 + 55), (unsigned char)(rand()%200 + 55), (unsigned char)(rand()%200 + 55), 255 };
        
        // ==========================================
        // FITUR BATASAN 6 MOBIL (MENCEGAH TUMPUKAN ANTRIAN)
        // ==========================================
        int countN = 0, countE = 0, countS = 0, countW = 0;
        for (const auto& v : daftarKendaraan) {
            if (v.getOrigin() == "N") countN++;
            else if (v.getOrigin() == "E") countE++;
            else if (v.getOrigin() == "S") countS++;
            else if (v.getOrigin() == "W") countW++;
        }

        // Tentukan arah mana yang akan di-spawn secara acak (0=Utara, 1=Timur, 2=Selatan, 3=Barat)
        int r = rand() % 4;
        
        // Spawn hanya jika mobil dari arah tersebut belum mencapai limit 6 unit di layar
        if (r == 0 && countN < 6) daftarKendaraan.push_back(Kendaraan({-4.0f, 0.5f, 45.0f}, {0.0f, 0.0f, -1.0f}, "N", c)); // Dari Utara
        else if (r == 1 && countE < 6) daftarKendaraan.push_back(Kendaraan({45.0f, 0.5f, 4.0f}, {-1.0f, 0.0f, 0.0f}, "E", c)); // Dari Timur
        else if (r == 2 && countS < 6) daftarKendaraan.push_back(Kendaraan({4.0f, 0.5f, -45.0f}, {0.0f, 0.0f, 1.0f}, "S", c)); // Dari Selatan
        else if (r == 3 && countW < 6) daftarKendaraan.push_back(Kendaraan({-45.0f, 0.5f, -4.0f}, {1.0f, 0.0f, 0.0f}, "W", c)); // Dari Barat
    }
}

// Fungsi Pengatur Waktu Dunia (Berjalan Setiap Detik)
void Persimpangan::perbaruiDunia(float dt, double jedaUtaraSelatan, double jedaTimurBarat) {
    timerLampu += dt;
    
    // Total waktu jatah (Hijau + Kuning) untuk satu arah jalan = (Total Siklus / 4)
    double phaseDuration = optimalCycleTime / 4.0;
    
    // Pastikan durasi lampu kuning proporsional (maksimum 2 detik atau 25% dari fase agar hijau tetap ada)
    double actualYellowDuration = std::fmin(2.0, phaseDuration * 0.25);
    
    if (!sedangFaseKuning) {
        // --- SEDANG FASE HIJAU ---
        // Jika sisa waktu fase kurang dari durasi kuning yang diizinkan, nyalakan lampu Kuning!
        if (timerLampu > (phaseDuration - actualYellowDuration)) {
            sedangFaseKuning = true;
        }
    } else {
        // --- SEDANG FASE KUNING ---
        // Cek jika fase kuning sudah selesai, maka beralih ke lampu merah (giliran fase arah selanjutnya)
        if (timerLampu > phaseDuration) {
            timerLampu = 0.0f; // Reset stopwatch
            sedangFaseKuning = false; // Kuning Mati
            faseSaatIni = (faseSaatIni + 1) % 4; // Berputar 0(N) -> 1(E) -> 2(S) -> 3(W) -> Kembali 0(N)
        }
    }

    // Panggil Spawner mobil
    munculkanKendaraan(dt, jedaUtaraSelatan, jedaTimurBarat);

    // Di perbaruiPerilaku, kita meneruskan informasi apakah sedang lampu kuning atau tidak ke otak mobil
    for (auto& v : daftarKendaraan) {
        v.perbaruiPerilaku(dt, faseSaatIni, daftarKendaraan, sedangFaseKuning);
    }

    // "Garbage Collector" manual: Hapus kendaraan dari RAM komputer jika sudah jauh keluar layar
    for (auto it = daftarKendaraan.begin(); it != daftarKendaraan.end(); ) {
        if (std::abs(it->getPosition().x) > 50.0f || std::abs(it->getPosition().z) > 50.0f) {
            it = daftarKendaraan.erase(it);
        } else {
            ++it;
        }
    }
}

// ==========================================
// PENGGAMBARAN MODEL 3D PERSIMPANGAN
// ==========================================
void Persimpangan::drawWorld3D() const {
    // 1. Gambar Permukaan Aspal Hitam/Abu Tua
    DrawCube(Vector3{0, 0, 0}, 16.0f, 0.1f, 100.0f, DARKGRAY); // Jalan Utara-Selatan (NS)
    DrawCube(Vector3{0, 0, 0}, 100.0f, 0.1f, 16.0f, DARKGRAY); // Jalan Timur-Barat (EW)
    
    // 2. Gambar Marka Jalan (Garis Putih Putus-Putus Pemisah Jalur Berlawanan)
    // Digambar HANYA JAUH SEBELUM persimpangan, tidak melintasi zebra cross agar rapi (Sesuai Aturan Real-Life)
    DrawCube(Vector3{0, 0.15f, 32.0f}, 0.3f, 0.1f, 36.0f, RAYWHITE);  // Marka Jalan Utara
    DrawCube(Vector3{0, 0.15f, -32.0f}, 0.3f, 0.1f, 36.0f, RAYWHITE); // Marka Jalan Selatan
    DrawCube(Vector3{32.0f, 0.15f, 0}, 36.0f, 0.1f, 0.3f, RAYWHITE);  // Marka Jalan Timur
    DrawCube(Vector3{-32.0f, 0.15f, 0}, 36.0f, 0.1f, 0.3f, RAYWHITE); // Marka Jalan Barat

    // 3. Zebra Cross (Balok Putih Kecil Melintang di 4 Sisi)
    for(int i=-6; i<=6; i+=2) {
        if (i == 0) continue; // Lewati bagian tengah persis
        DrawCube(Vector3{(float)i, 0.15f, 12.0f}, 1.0f, 0.1f, 3.0f, RAYWHITE); // Zebra Utara
        DrawCube(Vector3{(float)i, 0.15f, -12.0f}, 1.0f, 0.1f, 3.0f, RAYWHITE); // Zebra Selatan
        DrawCube(Vector3{12.0f, 0.15f, (float)i}, 3.0f, 0.1f, 1.0f, RAYWHITE); // Zebra Timur
        DrawCube(Vector3{-12.0f, 0.15f, (float)i}, 3.0f, 0.1f, 1.0f, RAYWHITE); // Zebra Barat
    }

    // 4. Garis Berhenti (Stop Line) - Tepat di belakang Zebra Cross
    DrawCube(Vector3{-4.0f, 0.15f, 14.0f}, 7.5f, 0.1f, 0.5f, RAYWHITE); // Stop Line Utara
    DrawCube(Vector3{4.0f, 0.15f, -14.0f}, 7.5f, 0.1f, 0.5f, RAYWHITE); // Stop Line Selatan
    DrawCube(Vector3{14.0f, 0.15f, 4.0f}, 0.5f, 0.1f, 7.5f, RAYWHITE);  // Stop Line Timur
    DrawCube(Vector3{-14.0f, 0.15f, -4.0f}, 0.5f, 0.1f, 7.5f, RAYWHITE); // Stop Line Barat

    // ==========================================
    // 5. MENGGAMBAR TIANG LAMPU LALU LINTAS
    // ==========================================
    // Koordinat penempatan ke-empat tiang di sudut jalan
    Vector3 polePos[4] = {
        {-9.0f, 0.0f, 14.0f},  // Tiang Sisi Utara
        {14.0f, 0.0f, 9.0f},   // Tiang Sisi Timur
        {9.0f, 0.0f, -14.0f},  // Tiang Sisi Selatan
        {-14.0f, 0.0f, -9.0f}  // Tiang Sisi Barat
    };

    for(int i=0; i<4; i++) {
        // Tiang Abu-Abu Solid
        DrawCube(Vector3{polePos[i].x, 3.0f, polePos[i].z}, 0.5f, 6.0f, 0.5f, GRAY);
        // Kotak Lampu Hitam (Housing) di ujung atas tiang
        DrawCube(Vector3{polePos[i].x, 6.5f, polePos[i].z}, 1.5f, 3.5f, 1.5f, BLACK);

        // Menentukan Lampu Warna Apa yang Menyala
        // Default semua redup (Abu Tua)
        Color colorTop = DARKGRAY, colorMid = DARKGRAY, colorBot = DARKGRAY;
        
        if (faseSaatIni == i) {
            if (sedangFaseKuning) colorMid = YELLOW; // Menyala Kuning Tengah
            else colorBot = GREEN;                // Menyala Hijau Bawah
        } else {
            colorTop = RED;                       // Menyala Merah Atas
        }

        // Geser sedikit ke depan wajah bohlam agar tidak tertutup kotak hitam housing
        float offsetZ = (i==0) ? -0.8f : (i==2) ? 0.8f : 0.0f;
        float offsetX = (i==1) ? -0.8f : (i==3) ? 0.8f : 0.0f;

        // Gambar 3 Bohlam Lampu
        DrawSphere(Vector3{polePos[i].x + offsetX, 7.5f, polePos[i].z + offsetZ}, 0.4f, colorTop); // Merah
        DrawSphere(Vector3{polePos[i].x + offsetX, 6.5f, polePos[i].z + offsetZ}, 0.4f, colorMid); // Kuning
        DrawSphere(Vector3{polePos[i].x + offsetX, 5.5f, polePos[i].z + offsetZ}, 0.4f, colorBot); // Hijau
    }

    // 6. Terakhir, perintahkan setiap mobil untuk menggambar dirinya sendiri
    for (const auto& v : daftarKendaraan) {
        v.gambar();
    }
}

// ==========================================
// PENGGAMBARAN ANTARMUKA PENGGUNA (UI) OVERLAY
// ==========================================
void Persimpangan::drawWorld2D(Camera camera) const {
    // Fungsi ini dipanggil setelah Mode 3D dihentikan
    // Kita menembakkan titik koordinat 3D ke Layar Datar (Screen 2D) untuk memunculkan Hitung Mundur

    // Koordinat maya di atas setiap tiang
    Vector3 textPos[4] = {
        {-9.0f, 9.0f, 14.0f},
        {14.0f, 9.0f, 9.0f},
        {9.0f, 9.0f, -14.0f},
        {-14.0f, 9.0f, -9.0f}
    };

    double phaseDuration = optimalCycleTime / 4.0;
    
    for(int i=0; i<4; i++) {
        // Ubah 3D World Space ke 2D Screen Space
        Vector2 screenPos = GetWorldToScreen(textPos[i], camera);
        
        // Hanya gambar jika layarnya berada di area pandang kamera 
        if (screenPos.x >= 0 && screenPos.x <= GetScreenWidth() && screenPos.y >= 0 && screenPos.y <= GetScreenHeight()) {
            
            std::string statusText;
            Color textColor = WHITE;
            
            // Hitung kalkulasi mundur angka (dalam detik fiktif)
            if (faseSaatIni == i) {
                if (sedangFaseKuning) {
                    // Sisa Kuning
                    int sisa = std::ceil(phaseDuration - timerLampu);
                    statusText = TextFormat("%d", sisa);
                    textColor = YELLOW;
                } else {
                    // Sisa Hijau
                    double actualYellowDuration = std::fmin(2.0, phaseDuration * 0.25);
                    int sisa = std::ceil((phaseDuration - actualYellowDuration) - timerLampu);
                    statusText = TextFormat("%d", sisa);
                    textColor = GREEN;
                }
            } else {
                // Kalkulasi rumit penantian Lampu Merah: 
                // Kita harus menghitung 'berapa fase giliran lagi' sampai kembali ke arah mobil ini (faseTunggu)
                int faseTunggu = (i - faseSaatIni + 4) % 4; 
                // Lalu dikali dengan durasi per fase, dikurang stopwatch yang sudah berjalan
                double waktuTunggu = (faseTunggu * phaseDuration) - timerLampu;
                int sisa = std::ceil(waktuTunggu);
                statusText = TextFormat("%d", sisa);
                textColor = RED;
            }

            // Gambar Shadow Hitam agar tulisan tidak menyatu dengan background putih
            DrawText(statusText.c_str(), screenPos.x - 18, screenPos.y - 18, 40, BLACK);
            // Gambar Text Berwarna yang sesungguhnya di atasnya
            DrawText(statusText.c_str(), screenPos.x - 20, screenPos.y - 20, 40, textColor);
        }
    }
}