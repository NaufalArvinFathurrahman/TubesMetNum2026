// ==========================================
// IMPLEMENTASI KECERDASAN KENDARAAN (Vehicle.cpp)
// ==========================================
// Berkas ini mengatur bagaimana cara mobil "berpikir".
// Mencakup logika kapan dia harus mengerem, berbelok, dan
// mendeteksi mobil lain agar tidak saling bertabrakan.

#include "Vehicle.h"
#include "raymath.h"
#include <cmath>
#include <cstdlib>

// Konstruktor: Menerima nilai awal saat spawner Intersection menciptakan objek ini
Vehicle::Vehicle(Vector3 startPos, Vector3 startDir, std::string startOrigin, Color vehicleColor) {
    position = startPos;
    direction = startDir;
    maxSpeed = 15.0f; // Batas kecepatan maksimal kendaraan
    currentSpeed = maxSpeed;
    origin = startOrigin;
    color = vehicleColor;
    
    // Secara acak menentukan arah yang akan diambil mobil di persimpangan
    // 0 = Jalan Terus (Lurus), 1 = Belok Kiri, 2 = Belok Kanan
    turnChoice = rand() % 3; 
    hasTurned = false; // Flag bahwa mobil belum mencapai titik tengah untuk membelok
}

// Fungsi utama Kecerdasan Buatan (AI) kendaraan
void Vehicle::updateBehavior(float dt, int currentPhase, const std::vector<Vehicle>& otherVehicles, bool isYellowPhase) {
    bool harus_berhenti = false; // Asumsi awal: Mobil bisa melaju bebas
    
    // ==========================================
    // 1. LOGIKA LAMPU LALU LINTAS
    // ==========================================
    
    // Mengecek apakah lampu lalu lintas untuk jalur mobil ini sedang mendapatkan giliran Hijau/Kuning
    bool isMyPhase = false;
    if (origin == "N" && currentPhase == 0) isMyPhase = true;
    if (origin == "E" && currentPhase == 1) isMyPhase = true;
    if (origin == "S" && currentPhase == 2) isMyPhase = true;
    if (origin == "W" && currentPhase == 3) isMyPhase = true;

    // Menentukan zona koordinat tepat di belakang Garis Berhenti (Zebra Cross)
    // Mobil harus ditahan di rentang posisi ini jika lampu merah
    bool atStopLine = false;
    if (origin == "N" && position.z > 15.0f && position.z < 20.0f) atStopLine = true;
    if (origin == "S" && position.z < -15.0f && position.z > -20.0f) atStopLine = true;
    if (origin == "E" && position.x > 15.0f && position.x < 20.0f) atStopLine = true;
    if (origin == "W" && position.x < -15.0f && position.x > -20.0f) atStopLine = true;

    // Jika sedang berada di garis berhenti dan lampunya BUKAN lampu hijau/kuning miliknya
    if (atStopLine && !isMyPhase) {
        harus_berhenti = true; // Wajib Rem
        
        // Aturan Khusus: Belok Kiri Langsung (Boleh Menerobos Lampu Merah dengan Hati-Hati)
        if (turnChoice == 1) {
            bool adaMobilMelintas = false;
            // Memeriksa area tengah persimpangan, apakah ada mobil dari arah lain yang sedang lewat
            for (const auto& other : otherVehicles) {
                if (&other != this && other.getOrigin() != origin) {
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
    if (isMyPhase && isYellowPhase) {
        bool farEnoughToStop = false;
        // Jarak deteksi diperjauh hingga 35 meter ke belakang agar punya waktu mengerem
        if (origin == "N" && position.z > 18.0f && position.z < 35.0f) farEnoughToStop = true;
        if (origin == "S" && position.z < -18.0f && position.z > -35.0f) farEnoughToStop = true;
        if (origin == "E" && position.x > 18.0f && position.x < 35.0f) farEnoughToStop = true;
        if (origin == "W" && position.x < -18.0f && position.x > -35.0f) farEnoughToStop = true;
        
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
            Vector3 toDepan = Vector3Subtract(depan.getPosition(), position);
            float dist = Vector3Length(toDepan); // Mengukur jarak riil dalam meter
            
            // Jika mobil 'depan' berada dalam radius berbahaya (kurang dari 7 meter)
            if (dist > 0.1f && dist < 7.0f) { 
                // Normalisasi vektor untuk mendapatkan murni 'arahnya saja'
                Vector3 toDepanNorm = Vector3Scale(toDepan, 1.0f / dist);
                // Menghitung produk titik (Dot Product) antara arah hadap saya dengan posisi mobil depan
                float dot = Vector3DotProduct(direction, toDepanNorm);
                
                // Jika hasil dot mendekati 1 (tepat di depan moncong mobil), aktifkan rem!
                if (dot > 0.8f) harus_berhenti = true; 
            }
        }
    }

    // ==========================================
    // 3. LOGIKA MEMBELOK (NAVIGASI)
    // ==========================================
    // Memutar arah vektor laju mobil saat mencapai titik koordinat tertentu di tengah perempatan
    if (!hasTurned) {
        bool triggerTurn = false;
        if (turnChoice == 1) { // --- Belok Kiri (Memotong Jalur Dalam) ---
            if (origin == "N" && position.z <= -4.0f) { direction = {1,0,0}; position.z = -4.0f; triggerTurn = true; }
            if (origin == "S" && position.z >= 4.0f)  { direction = {-1,0,0}; position.z = 4.0f; triggerTurn = true; }
            if (origin == "E" && position.x <= -4.0f) { direction = {0,0,-1}; position.x = -4.0f; triggerTurn = true; }
            if (origin == "W" && position.x >= 4.0f)  { direction = {0,0,1}; position.x = 4.0f; triggerTurn = true; }
        } 
        else if (turnChoice == 2) { // --- Belok Kanan (Memotong Jalur Luar) ---
            if (origin == "N" && position.z <= 4.0f) { direction = {-1,0,0}; position.z = 4.0f; triggerTurn = true; }
            if (origin == "S" && position.z >= -4.0f){ direction = {1,0,0}; position.z = -4.0f; triggerTurn = true; }
            if (origin == "E" && position.x <= 4.0f) { direction = {0,0,1}; position.x = 4.0f; triggerTurn = true; }
            if (origin == "W" && position.x >= -4.0f){ direction = {0,0,-1}; position.x = -4.0f; triggerTurn = true; }
        }
        if (triggerTurn) hasTurned = true; // Tandai bahwa navigasi putar sudah selesai
    }

    // ==========================================
    // 4. EKSEKUSI FISIKA & PERGERAKAN (KINEMATIKA)
    // ==========================================
    if (harus_berhenti) {
        // Pengereman (Deselerasi lambat namun kuat)
        currentSpeed -= 40.0f * dt; 
        if (currentSpeed < 0) currentSpeed = 0; // Tidak bisa mundur
    } else {
        // Menambah Gas (Akselerasi)
        currentSpeed += 15.0f * dt; 
        if (currentSpeed > maxSpeed) currentSpeed = maxSpeed; // Limit ke top-speed
        
        // Pengereman Halus Khusus Tikungan: Memperlambat kecepatan saat masuk area persimpangan
        if (std::abs(position.x) < 15.0f && std::abs(position.z) < 15.0f) {
            if (currentSpeed > 8.0f) currentSpeed = 8.0f; // Pelan saat berbelok
        }
    }

    // Mengubah Posisi menggunakan formula Kinematika Fisika: Jarak = Kecepatan * Waktu
    position.x += direction.x * currentSpeed * dt;
    position.z += direction.z * currentSpeed * dt;
}

// Fungsi menggambar 3D objek kendaraan
void Vehicle::draw() const {
    // Mobil akan berbentuk balok panjang (mengikuti arah hadap sumbu-Z atau sumbu-X)
    Vector3 size = (std::abs(direction.z) > 0.5f) ? Vector3{2.0f, 1.5f, 4.0f} : Vector3{4.0f, 1.5f, 2.0f};
    
    // Gambar balok solid bewarna
    DrawCube(position, size.x, size.y, size.z, color);
    
    // Gambar garis kerangka hitam (Wireframe) untuk mempertegas bentuk
    DrawCubeWires(position, size.x, size.y, size.z, BLACK);
    
    // ==========================================
    // ANIMASI LAMPU SEIN (BLINKING INDICATOR)
    // ==========================================
    // Menyala kuning/oranye di atas atap jika mobil mendekati persimpangan dan berencana membelok
    if (!hasTurned && std::abs(position.x) < 25.0f && std::abs(position.z) < 25.0f) {
        if (turnChoice == 1) 
            DrawCube(Vector3Add(position, Vector3{0, 1.0f, 0}), 1.0f, 0.5f, 1.0f, YELLOW); // Sein Kiri
        else if (turnChoice == 2) 
            DrawCube(Vector3Add(position, Vector3{0, 1.0f, 0}), 1.0f, 0.5f, 1.0f, ORANGE); // Sein Kanan
    }
}