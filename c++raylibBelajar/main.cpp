// ==========================================
// PROGRAM SIMULASI LALU LINTAS - UTAMA (main.cpp)
// ==========================================
// Berkas ini adalah titik masuk (entry point) dari program.
// Fungsinya adalah menginisialisasi jendela (window), mengatur kamera,
// menjalankan perhitungan matematika pertama kali, serta menampung "Game Loop" utama.

#include "raylib.h"
#include "MesinMatematika.h"
#include "Persimpangan.h"

// Fungsi ini digunakan untuk menghitung ulang kepadatan lalu lintas.
// Fungsi ini dipanggil setiap kali keadaan "Jam Sibuk" / "Jam Sepi" berubah.
void recalculateTraffic(bool sedangJamSibuk, double& jedaUtaraSelatan, double& jedaTimurBarat, double& siklusVisual, double& siklusMatematis) {
    // Matriks A merepresentasikan rasio aliran kendaraan (persamaan linear)
    std::vector<std::vector<double>> Matriks_A = {
        { 4, -1,  0, -1}, {-1,  4, -1,  0}, { 0, -1,  4, -1}, {-1,  0, -1,  3}
    };
    
    // Vektor B merepresentasikan volume kepadatan kendaraan yang datang dari luar
    std::vector<double> Vektor_B;
    if (sedangJamSibuk) {
        Vektor_B = {120, 90, 150, 110}; // Kepadatan tinggi
    } else {
        Vektor_B = {30, 20, 40, 25}; // Kepadatan turun drastis
    }

    // --- Langkah 1: Dekomposisi LU ---
    // Memecah matriks menjadi Lower (L) dan Upper (U) untuk mencari nilai Vektor X (flow mentah)
    std::vector<std::vector<double>> L, U;
    MesinMatematika::dekomposisi_lu(Matriks_A, L, U);
    std::vector<double> flow_mentah = MesinMatematika::selesaikan_lu(L, U, Vektor_B);

    // --- Langkah 2: Hitung Total Flow ---
    double sum_flow = 0;
    for (double f : flow_mentah) sum_flow += f;
    
    // Parameter turunan untuk mencari delay minimum
    double param_a = sum_flow * 0.05;
    double param_b = sum_flow * 150.0;
    
    // --- Langkah 3: Optimasi Newton-Raphson ---
    // Menggunakan metode pencarian akar untuk mendapatkan siklus lampu lalu lintas yang paling optimal
    siklusMatematis = MesinMatematika::newton_raphson(60.0, param_a, param_b);

    // --- Langkah 4: Hitung Interval Spawn ---
    // Menghitung berapa detik sekali mobil baru harus dimunculkan ke layar
    if (sedangJamSibuk) {
        // Pada jam sibuk, mobil sering muncul (interval kecil)
        jedaUtaraSelatan = (60.0 / ((flow_mentah[0] + flow_mentah[2]) / 2)) * 0.5;
        jedaTimurBarat = (60.0 / ((flow_mentah[1] + flow_mentah[3]) / 2)) * 0.5;
    } else {
        // Pada jam sepi, mobil jarang muncul (interval besar)
        jedaUtaraSelatan = (60.0 / ((flow_mentah[0] + flow_mentah[2]) / 2)) * 4.0;
        jedaTimurBarat = (60.0 / ((flow_mentah[1] + flow_mentah[3]) / 2)) * 4.0;
    }
    
    // Sesuaikan durasi siklus matematis untuk kebutuhan animasi visual agar tidak membosankan
    siklusVisual = siklusMatematis / 5.0; 
    if (siklusVisual < 10.0) siklusVisual = 10.0; // Minimal siklus visual adalah 10 detik
}

int main() {
    // 1. OTOMATISASI METODE NUMERIK AWAL
    bool sedangJamSibuk = false; // Memulai simulasi pada jam 06:00 pagi (Jam Sepi)
    double interval_ns, interval_ew, siklus_visual, siklus_matematis;
    
    // Lakukan perhitungan pertama kalinya
    recalculateTraffic(sedangJamSibuk, interval_ns, interval_ew, siklus_visual, siklus_matematis);

    // 2. INISIALISASI ENGINE VISUAL (RAYLIB)
    InitWindow(1000, 800, "Simulasi Lalu Lintas - OOP Raylib (Extended)");
    SetTargetFPS(60); // Batasi frame per detik menjadi 60 FPS
    
    // Setup Kamera 3D Isometrik (Melihat dari atas menyudut)
    Camera camera = { 0 };
    camera.position = Vector3{ 25.0f, 30.0f, -25.0f }; // Posisi Kamera
    camera.target = Vector3{ 0.0f, 0.0f, 0.0f };       // Titik Fokus Kamera (tengah persimpangan)
    camera.up = Vector3{ 0.0f, 1.0f, 0.0f };           // Arah Atas
    camera.fovy = 45.0f;                               // Field of View
    camera.projection = CAMERA_PERSPECTIVE;
    
    // Inisialisasi dunia persimpangan
    Persimpangan dunia(siklus_visual);

    // Variabel waktu simulasi 24-Jam
    int simulatedHours = 6;         // Mulai dari Pukul 06
    float simulatedMinutes = 0.0f;  // Mulai dari Menit 00

    // 3. GAME LOOP (Perulangan Utama)
    // Akan terus berjalan selama jendela tidak ditutup
    while (!WindowShouldClose()) {
        float dt = GetFrameTime(); // Waktu yang berlalu sejak frame terakhir
        
        // --- LOGIKA WAKTU SIMULASI ---
        // 1 Detik dunia nyata disimulasikan sebagai 2 Menit dalam simulasi
        simulatedMinutes += dt * 2.0f;
        
        // Kontrol Manual: Jika panah kanan ditekan, lewati waktu 1 Jam (tambah 60 Menit)
        if (IsKeyPressed(KEY_RIGHT)) {
            simulatedMinutes += 60.0f; 
        }
        
        // Cek jika menit sudah mencapai atau melewati 60
        if (simulatedMinutes >= 60.0f) {
            simulatedMinutes -= 60.0f; // Reset sisa menit
            simulatedHours++;          // Tambah 1 Jam
            if (simulatedHours >= 24) simulatedHours = 0; // Ulangi ke jam 0 (tengah malam)
            
            // --- EVALUASI JADWAL JAM SIBUK ---
            bool nowSibuk = false;
            // Jam sibuk: Pagi (07-09), Siang (11-13), Sore (16-18)
            if ((simulatedHours >= 7 && simulatedHours < 9) || 
                (simulatedHours >= 11 && simulatedHours < 13) || 
                (simulatedHours >= 16 && simulatedHours < 18)) {
                nowSibuk = true;
            }
            
            // Jika status jam berubah (dari Sibuk ke Sepi atau sebaliknya)
            if (nowSibuk != sedangJamSibuk) {
                sedangJamSibuk = nowSibuk;
                // Hitung ulang semua jadwal lampu dan kedatangan mobil
                recalculateTraffic(sedangJamSibuk, interval_ns, interval_ew, siklus_visual, siklus_matematis);
                dunia.setOptimalCycleTime(siklus_visual);
            }
        }

        // Perbarui semua objek dalam dunia (mobil, lampu lalu lintas)
        dunia.perbaruiDunia(dt, interval_ns, interval_ew);

        // --- MULAI MENGGAMBAR KE LAYAR (RENDERING) ---
        BeginDrawing();
        ClearBackground(RAYWHITE); // Background warna putih abu-abu
        
        // Lapisan 1: Gambar Dunia 3D
        BeginMode3D(camera);
        dunia.drawWorld3D();
        EndMode3D();
        
        // Lapisan 2: Gambar Teks UI yang menempel di 3D (Text Countdown Lampu)
        dunia.drawWorld2D(camera);
        
        // Lapisan 3: Gambar HUD Text (Informasi Layar di Pojok Kiri Atas)
        DrawRectangle(5, 5, 450, 100, Fade(WHITE, 0.8f)); // Kotak transparan
        DrawText(TextFormat("Jam Simulasi: %02d:%02d", simulatedHours, (int)simulatedMinutes), 10, 10, 25, DARKGREEN);
        DrawText(sedangJamSibuk ? "Status: JAM SIBUK" : "Status: JAM SEPI", 10, 40, 20, sedangJamSibuk ? RED : BLUE);
        DrawText(TextFormat("Siklus Kalkulasi: %.2f dtk", siklus_matematis), 10, 65, 15, BLACK);
        DrawText("Tekan [PANAH KANAN] untuk maju 1 Jam", 10, 85, 15, DARKGRAY);
        
        EndDrawing();
    }

    // Bebaskan memory dan tutup jendela
    CloseWindow();
    return 0;
}