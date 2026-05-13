# ==========================================
# PROGRAM SIMULASI LALU LINTAS - VERSI PYTHON (Tubes.py)
# ==========================================
# Program ini mensimulasikan lalu lintas 4 arah (Perempatan) dengan basis 3D.
# Alih-alih angka acak, pengaturan pergantian waktu lampu merah dan hijau 
# dikalkulasikan secara matematis (Numerik) untuk mendapatkan efisiensi (Siklus Optimum).

from ursina import *
import random

# ==========================================
# 1. OTAK MATEMATIKA: DEKOMPOSISI LU & NEWTON-RAPHSON
# ==========================================

# Fungsi ini memecah Matriks A menjadi dua matriks: Lower (L) dan Upper (U).
# Berguna untuk menyelesaikan persamaan linier secara efisien oleh komputer.
def lu_decomposition(A):
    n = len(A)
    L = [[0.0] * n for _ in range(n)] # Inisialisasi Matriks L berisi angka 0
    U = [[0.0] * n for _ in range(n)] # Inisialisasi Matriks U berisi angka 0

    for i in range(n):
        # Membangun Matriks Atas (Upper Triangular)
        for k in range(i, n):
            sum_upper = sum(L[i][j] * U[j][k] for j in range(i))
            U[i][k] = A[i][k] - sum_upper
            
        # Membangun Matriks Bawah (Lower Triangular)
        for k in range(i, n):
            if i == k:
                L[i][i] = 1.0 # Garis miring utama (diagonal) matriks L selalu 1
            else:
                sum_lower = sum(L[k][j] * U[j][i] for j in range(i))
                L[k][i] = (A[k][i] - sum_lower) / U[i][i]
    return L, U

# Fungsi untuk mencari Vektor Flow Aktual (Berapa mobil yang melintas sesungguhnya per menit)
def solve_lu(L, U, B):
    n = len(L)
    Y = [0.0] * n # Matriks penengah (Dummy)
    X = [0.0] * n # Matriks hasil akhir (Arus)
    
    # Substitusi Maju (Cari Y)
    for i in range(n):
        sum_y = sum(L[i][j] * Y[j] for j in range(i))
        Y[i] = B[i] - sum_y
        
    # Substitusi Mundur (Cari X dari Y)
    for i in range(n - 1, -1, -1):
        sum_x = sum(U[i][j] * X[j] for j in range(i + 1, n))
        X[i] = (Y[i] - sum_x) / U[i][i]
    return X

# Rumus Fungsi Tundaan Total Lalu Lintas
def g(C, a, b):
    return a - (b / (C**2))

# Rumus Turunan Pertama Fungsi Tundaan
def g_prime(C, b):
    return 2 * b / (C**3)

# Algoritma Pencarian Akar Persamaan (Untuk menemukan Durasi Siklus Lampu yang Paling Optimal)
def newton_raphson_optimal_cycle(initial_guess, a, b, tolerance=1e-6, max_iter=100):
    C_current = initial_guess # Tebakan awal (Misal: 60 detik)
    print("\n--- Memulai Kalkulasi Otak Matematika ---")
    
    for i in range(max_iter):
        g_val = g(C_current, a, b)
        g_prime_val = g_prime(C_current, b)
        
        # Mencegah program crash kalau terjadi error pembagian dengan nol
        if abs(g_prime_val) < 1e-10: break
        
        # Eksekusi Rumus Dasar Newton Raphson: X_baru = X_lama - ( f(x) / f'(x) )
        C_next = C_current - (g_val / g_prime_val)
        
        # Jika nilai baru dan nilai lama sudah sangat mirip (selisih lebih kecil dari toleransi)
        if abs(C_next - C_current) < tolerance:
            print(f"Konvergensi Newton-Raphson berhasil: Siklus {C_next:.2f} detik")
            return C_next
            
        # Update tebakan untuk iterasi berikutnya
        C_current = C_next
    return C_current


# ==========================================
# 2. INISIALISASI ENGINE GRAFIS (URSINA)
# ==========================================
# Menyiapkan Canvas/Jendela tempat dunia 3D akan digambar
app = Ursina(title="Simulasi Lalu Lintas - Optimasi Numerik")


# ==========================================
# 3. MENGHUBUNGKAN MATEMATIKA KE SIMULASI
# ==========================================
# A. Eksekusi Matriks
Matriks_A = [[4, -1, 0, -1], [-1, 4, -1, 0], [0, -1, 4, -1], [-1, 0, -1, 3]]
Vektor_B = [120, 90, 150, 110] # Parameter kemacetan dari 4 arah
L, U = lu_decomposition(Matriks_A)
flow_mentah = solve_lu(L, U, Vektor_B) # Diperoleh List: Jumlah riil mobil per arah

# B. Eksekusi Newton-Raphson
param_a = sum(flow_mentah) * 0.05
param_b = sum(flow_mentah) * 150.0 
siklus_matematis = newton_raphson_optimal_cycle(60.0, param_a, param_b)

# C. Konversi Nilai untuk Grafis 3D
# Siklus dipercepat untuk kebutuhan visual presentasi agar lampu cepat berganti
siklus_visual = max(10.0, siklus_matematis / 5.0) 

# Mengubah hasil matriks (kendaraan/menit) menjadi interval (detik kemunculan antar mobil)
rata_ns = (flow_mentah[0] + flow_mentah[2]) / 2 # Utara & Selatan
rata_ew = (flow_mentah[1] + flow_mentah[3]) / 2 # Timur & Barat

interval_ns = (60.0 / rata_ns) * 3 # Dikali 3 agar jalan tidak terlalu sesak di layar
interval_ew = (60.0 / rata_ew) * 3
flow_visual = [interval_ns, interval_ew]

# State memori Lampu (Lampu mana yang sedang hidup?)
status_lampu = {'NS': 'Hijau', 'EW': 'Merah'}
timer_lampu = 0.0


# ==========================================
# 4. MEMBANGUN DUNIA 3D
# ==========================================
# Membuat Aspal dengan balok panjang tipis berwarna abu gelap
jalan_ns = Entity(model='cube', scale=(8, 0.1, 60), color=color.dark_gray, position=(0,0,0))
jalan_ew = Entity(model='cube', scale=(60, 0.1, 8), color=color.dark_gray, position=(0,0,0))

# Membuat Lampu Indikator bulat
indikator_ns = Entity(model='sphere', scale=1.5, color=color.green, position=(0, 4, 3))
indikator_ew = Entity(model='sphere', scale=1.5, color=color.red, position=(3, 4, 0))

# Mengatur letak mata kita (Kamera) menghadap ke tengah persimpangan
camera.position = (20, 25, -20)
camera.look_at((0, 0, 0))


# ==========================================
# 5. KESADARAN AI KENDARAAN (Class / Objek)
# ==========================================
class KendaraanAI(Entity):
    # Dijalankan sekali saat mobil ini lahir
    def __init__(self, jalur, posisi_awal, rotasi_y):
        super().__init__(
            model='cube', 
            color=color.random_color(), 
            scale=(1.5, 1, 3), 
            position=posisi_awal,
            rotation_y=rotasi_y,
            collider='box' # Fisika tabrakan sederhana
        )
        self.jalur = jalur
        self.kecepatan_maks = random.uniform(8.0, 12.0) # Mobil bisa laju atau santai
        self.kecepatan_sekarang = self.kecepatan_maks
        self.jarak_aman = 4.0

    # Dijalankan terus menerus setiap FPS (Mendeteksi rem / tabrakan / lampu lalu lintas)
    def update(self):
        # Sensor Anti Tabrak: Raycast (Menembakkan sinar imajiner ke depan mobil)
        sensor_depan = raycast(self.position, self.forward, distance=self.jarak_aman, ignore=(self,))
        harus_berhenti_lampu = False
        
        # Mengecek apakah mobil sedang masuk batas pengereman persimpangan & lampu merah
        if self.jalur == 'NS':
            if abs(self.z) < 7 and abs(self.z) > 4 and status_lampu['NS'] == 'Merah':
                if (self.z > 0 and self.forward[2] < 0) or (self.z < 0 and self.forward[2] > 0):
                    harus_berhenti_lampu = True
        elif self.jalur == 'EW':
            if abs(self.x) < 7 and abs(self.x) > 4 and status_lampu['EW'] == 'Merah':
                if (self.x > 0 and self.forward[0] < 0) or (self.x < 0 and self.forward[0] > 0):
                    harus_berhenti_lampu = True

        # Eksekusi Pengereman (Deselerasi) menggunakan metode LERP (Linear Interpolation) untuk kemulusan
        if sensor_depan.hit or harus_berhenti_lampu:
            self.kecepatan_sekarang = lerp(self.kecepatan_sekarang, 0, time.dt * 5)
        else:
            self.kecepatan_sekarang = lerp(self.kecepatan_sekarang, self.kecepatan_maks, time.dt * 2)

        # Ubah posisi fisik ke depan berdasarkan kecepatan yang sedang ia miliki
        self.position += self.forward * self.kecepatan_sekarang * time.dt
        
        # Hapus/Bunuh objek mobil dari memory komputer jika ia sudah keluar area map
        if abs(self.x) > 40 or abs(self.z) > 40:
            destroy(self)


# ==========================================
# 6. PENGATUR LALU LINTAS UTAMA
# ==========================================
timer_spawn_ns = 0.0
timer_spawn_ew = 0.0

# Fungsi Utama Engine Ursina (Dipanggil 60 kali tiap 1 detik / 60 FPS)
def update():
    global timer_lampu, status_lampu, timer_spawn_ns, timer_spawn_ew
    
    # --- PENGATUR WAKTU LAMPU LALU LINTAS ---
    # Diatur berdasarkan durasi hasil algoritma Newton-Raphson
    timer_lampu += time.dt
    if timer_lampu > (siklus_visual / 2): # Jika waktunya habis
        timer_lampu = 0.0
        # Ganti warna / Gilir jalan
        if status_lampu['NS'] == 'Hijau':
            status_lampu['NS'] = 'Merah'
            status_lampu['EW'] = 'Hijau'
            indikator_ns.color = color.red
            indikator_ew.color = color.green
        else:
            status_lampu['NS'] = 'Hijau'
            status_lampu['EW'] = 'Merah'
            indikator_ns.color = color.green
            indikator_ew.color = color.red

    # --- PENGATUR SPAWNER (PEMUNCUL) KENDARAAN ---
    # Diatur berdasarkan jumlah Flow Kendaraan hasil algoritma Dekomposisi LU
    timer_spawn_ns += time.dt
    timer_spawn_ew += time.dt
    
    if timer_spawn_ns > flow_visual[0]:
        timer_spawn_ns = 0.0
        arah = random.choice([1, -1]) 
        pos_z = -30 * arah
        pos_x = 2 * arah 
        KendaraanAI('NS', (pos_x, 0.5, pos_z), rotasi_y=0 if arah == 1 else 180)
        
    if timer_spawn_ew > flow_visual[1]:
        timer_spawn_ew = 0.0
        arah = random.choice([1, -1]) 
        pos_x = -30 * arah
        pos_z = -2 * arah 
        KendaraanAI('EW', (pos_x, 0.5, pos_z), rotasi_y=90 if arah == 1 else -90)

# ==========================================
# 7. MULAI JALANKAN PROGRAM (APP START)
# ==========================================
app.run()