// ==========================================
// IMPLEMENTASI KECERDASAN KENDARAAN (Kendaraan.cpp)
// ==========================================
// Berkas ini mengatur bagaimana cara mobil "berpikir".
// Mencakup logika kapan dia harus mengerem, berbelok, dan
// mendeteksi mobil lain agar tidak saling bertabrakan.

#include "Kendaraan.h"
#include "raymath.h"
#include <cmath>
#include <cstdlib>

// Konstruktor: Menerima nilai awal saat spawner Persimpangan menciptakan objek ini
Kendaraan::Kendaraan(Vector3 startPos, Vector3 startDir, std::string startOrigin, Color vehicleColor) {
    posisi = startPos;
    arah = startDir;
    kecepatanMaks = 15.0f; // Batas kecepatan maksimal kendaraan
    kecepatanSaatIni = kecepatanMaks;
    asal = startOrigin;
    warna = vehicleColor;
    
    // Secara acak menentukan arah yang akan diambil mobil di persimpangan
    // 0 = Jalan Terus (Lurus), 1 = Belok Kiri, 2 = Belok Kanan
    pilihanBelok = rand() % 3; 
    sudahBelok = false; // Flag bahwa mobil belum mencapai titik tengah untuk membelok
}

// Fungsi utama Kecerdasan Buatan (AI) kendaraan
void Kendaraan::perbaruiPerilaku(float dt, int faseSaatIni, const std::vector<Kendaraan>& otherVehicles, bool sedangFaseKuning) {
    bool harus_berhenti = false; // Asumsi awal: Mobil bisa melaju bebas
    
    // ==========================================
    // 1. LOGIKA LAMPU LALU LINTAS
    // ==========================================
    
    // Mengecek apakah lampu lalu lintas untuk jalur mobil ini sedang mendapatkan giliran Hijau/Kuning
    bool isMyPhase = false;
    if (asal == "N" && faseSaatIni == 0) isMyPhase = true;
    if (asal == "E" && faseSaatIni == 1) isMyPhase = true;
    if (asal == "S" && faseSaatIni == 2) isMyPhase = true;
    if (asal == "W" && faseSaatIni == 3) isMyPhase = true;

    // Menentukan zona koordinat tepat di belakang Garis Berhenti (Zebra Cross)
    // Mobil harus ditahan di rentang posisi ini jika lampu merah
    bool atStopLine = false;
    if (asal == "N" && posisi.z > 15.0f && posisi.z < 20.0f) atStopLine = true;
    if (asal == "S" && posisi.z < -15.0f && posisi.z > -20.0f) atStopLine = true;
    if (asal == "E" && posisi.x > 15.0f && posisi.x < 20.0f) atStopLine = true;
    if (asal == "W" && posisi.x < -15.0f && posisi.x > -20.0f) atStopLine = true;

    // Jika sedang berada di garis berhenti dan lampunya BUKAN lampu hijau/kuning miliknya
    if (atStopLine && !isMyPhase) {
        harus_berhenti = true; // Wajib Rem
        
        // Aturan Khusus: Belok Kiri Langsung (Boleh Menerobos Lampu Merah dengan Hati-Hati)
        if (pilihanBelok == 1) {
            bool adaMobilMelintas = false;
            // Memeriksa area tengah persimpangan, apakah ada mobil dari arah lain yang sedang lewat
            for (const auto& other : otherVehicles) {
                if (&other != this && other.getOrigin() != asal) {
                    // Cek jika ada mobil asing di dalam kotak persimpangan (15x15 meter)
                    if (std::abs(other.getPosition().x) < 15.0f && std::abs(other.getPosition().z) < 15.0f) {
                        if (other.getSpeed() > 2.0f) { // Mobil tersebut sedang bergerak cepat
                            adaMobilMelintas = true;
                            break;
                        }
                    }
                }
            }
            // Jika kosong melompong, mobil diperbolehkan tidak mengerem (menerobos belok kiri)
            if (!adaMobilMelintas) harus_berhenti = false; 
        }
    }

    // Logika Lampu Kuning (Zona Pengereman Mendadak)
    // Jika lampu tiba-tiba kuning dan mobil masih jauh dari persimpangan, mobil wajib berhenti
    if (isMyPhase && sedangFaseKuning) {
        bool farEnoughToStop = false;
        // Jarak deteksi diperjauh hingga 35 meter ke belakang agar punya waktu mengerem
        if (asal == "N" && posisi.z > 18.0f && posisi.z < 35.0f) farEnoughToStop = true;
        if (asal == "S" && posisi.z < -18.0f && posisi.z > -35.0f) farEnoughToStop = true;
        if (asal == "E" && posisi.x > 18.0f && posisi.x < 35.0f) farEnoughToStop = true;
        if (asal == "W" && posisi.x < -18.0f && posisi.x > -35.0f) farEnoughToStop = true;
        
        if (farEnoughToStop) harus_berhenti = true;
        // Jika mobil sudah terlanjur melewati batas 18 meter, farEnoughToStop akan false
        // Artinya mobil akan menerobos maju untuk "menyelamatkan diri" / membersihkan persimpangan.
    }

    // ==========================================
    // 2. LOGIKA ANTI-TABRAKAN (COLLISION AVOIDANCE)
    // ==========================================
    // Menggunakan operasi matematika vektor (Dot Product)
    for (const auto& depan : otherVehicles) {
        if (&depan != this) {
            // Vektor jarak dari mobil saya ke mobil 'depan'
            Vector3 toDepan = Vector3Subtract(depan.getPosition(), posisi);
            float dist = Vector3Length(toDepan); // Mengukur jarak riil dalam meter
            
            // Jika mobil 'depan' berada dalam radius berbahaya (kurang dari 7 meter)
            if (dist > 0.1f && dist < 7.0f) { 
                // Normalisasi vektor untuk mendapatkan murni 'arahnya saja'
                Vector3 toDepanNorm = Vector3Scale(toDepan, 1.0f / dist);
                // Menghitung produk titik (Dot Product) antara arah hadap saya dengan posisi mobil depan
                float dot = Vector3DotProduct(arah, toDepanNorm);
                
                // Jika hasil dot mendekati 1 (tepat di depan moncong mobil), aktifkan rem!
                if (dot > 0.8f) harus_berhenti = true; 
            }
        }
    }

    // ==========================================
    // 3. LOGIKA MEMBELOK (NAVIGASI)
    // ==========================================
    // Memutar arah vektor laju mobil saat mencapai titik koordinat tertentu di tengah perempatan
    if (!sudahBelok) {
        bool triggerTurn = false;
        if (pilihanBelok == 1) { // --- Belok Kiri (Memotong Jalur Dalam) ---
            if (asal == "N" && posisi.z <= -4.0f) { arah = {1,0,0}; posisi.z = -4.0f; triggerTurn = true; }
            if (asal == "S" && posisi.z >= 4.0f)  { arah = {-1,0,0}; posisi.z = 4.0f; triggerTurn = true; }
            if (asal == "E" && posisi.x <= -4.0f) { arah = {0,0,-1}; posisi.x = -4.0f; triggerTurn = true; }
            if (asal == "W" && posisi.x >= 4.0f)  { arah = {0,0,1}; posisi.x = 4.0f; triggerTurn = true; }
        } 
        else if (pilihanBelok == 2) { // --- Belok Kanan (Memotong Jalur Luar) ---
            if (asal == "N" && posisi.z <= 4.0f) { arah = {-1,0,0}; posisi.z = 4.0f; triggerTurn = true; }
            if (asal == "S" && posisi.z >= -4.0f){ arah = {1,0,0}; posisi.z = -4.0f; triggerTurn = true; }
            if (asal == "E" && posisi.x <= 4.0f) { arah = {0,0,1}; posisi.x = 4.0f; triggerTurn = true; }
            if (asal == "W" && posisi.x >= -4.0f){ arah = {0,0,-1}; posisi.x = -4.0f; triggerTurn = true; }
        }
        if (triggerTurn) sudahBelok = true; // Tandai bahwa navigasi putar sudah selesai
    }

    // ==========================================
    // 4. EKSEKUSI FISIKA & PERGERAKAN (KINEMATIKA)
    // ==========================================
    if (harus_berhenti) {
        // Pengereman (Deselerasi lambat namun kuat)
        kecepatanSaatIni -= 40.0f * dt; 
        if (kecepatanSaatIni < 0) kecepatanSaatIni = 0; // Tidak bisa mundur
    } else {
        // Menambah Gas (Akselerasi)
        kecepatanSaatIni += 15.0f * dt; 
        if (kecepatanSaatIni > kecepatanMaks) kecepatanSaatIni = kecepatanMaks; // Limit ke top-speed
        
        // Pengereman Halus Khusus Tikungan: Memperlambat kecepatan saat masuk area persimpangan
        if (std::abs(posisi.x) < 15.0f && std::abs(posisi.z) < 15.0f) {
            if (kecepatanSaatIni > 8.0f) kecepatanSaatIni = 8.0f; // Pelan saat berbelok
        }
    }

    // Mengubah Posisi menggunakan formula Kinematika Fisika: Jarak = Kecepatan * Waktu
    posisi.x += arah.x * kecepatanSaatIni * dt;
    posisi.z += arah.z * kecepatanSaatIni * dt;
}

// Fungsi menggambar 3D objek kendaraan
void Kendaraan::gambar() const {
    // Mobil akan berbentuk balok panjang (mengikuti arah hadap sumbu-Z atau sumbu-X)
    Vector3 size = (std::abs(arah.z) > 0.5f) ? Vector3{2.0f, 1.5f, 4.0f} : Vector3{4.0f, 1.5f, 2.0f};
    
    // Gambar balok solid bewarna
    DrawCube(posisi, size.x, size.y, size.z, warna);
    
    // Gambar garis kerangka hitam (Wireframe) untuk mempertegas bentuk
    DrawCubeWires(posisi, size.x, size.y, size.z, BLACK);
    
    // ==========================================
    // ANIMASI LAMPU SEIN (BLINKING INDICATOR)
    // ==========================================
    // Menyala kuning/oranye di atas atap jika mobil mendekati persimpangan dan berencana membelok
    if (!sudahBelok && std::abs(posisi.x) < 25.0f && std::abs(posisi.z) < 25.0f) {
        if (pilihanBelok == 1) 
            DrawCube(Vector3Add(posisi, Vector3{0, 1.0f, 0}), 1.0f, 0.5f, 1.0f, YELLOW); // Sein Kiri
        else if (pilihanBelok == 2) 
            DrawCube(Vector3Add(posisi, Vector3{0, 1.0f, 0}), 1.0f, 0.5f, 1.0f, ORANGE); // Sein Kanan
    }
}