import pygame
import math
import random
import sys

# ==========================================
# 1. MESIN MATEMATIKA (Dekomposisi LU & Newton-Raphson)
# ==========================================
def lu_decomposition(A):
    n = len(A)
    L = [[0.0] * n for _ in range(n)]
    U = [[0.0] * n for _ in range(n)]

    for i in range(n):
        for k in range(i, n):
            sum_upper = sum(L[i][j] * U[j][k] for j in range(i))
            U[i][k] = A[i][k] - sum_upper
        for k in range(i, n):
            if i == k:
                L[i][i] = 1.0
            else:
                sum_lower = sum(L[k][j] * U[j][i] for j in range(i))
                L[k][i] = (A[k][i] - sum_lower) / U[i][i]
    return L, U

def solve_lu(L, U, B):
    n = len(L)
    Y = [0.0] * n
    X = [0.0] * n
    for i in range(n):
        sum_y = sum(L[i][j] * Y[j] for j in range(i))
        Y[i] = B[i] - sum_y
    for i in range(n - 1, -1, -1):
        sum_x = sum(U[i][j] * X[j] for j in range(i + 1, n))
        X[i] = (Y[i] - sum_x) / U[i][i]
    return X

def newton_raphson(initial_guess, a, b):
    C_current = initial_guess
    for _ in range(100):
        g_val = a - (b / (C_current**2))
        g_prime_val = 2 * b / (C_current**3)
        if abs(g_prime_val) < 1e-10: break
        C_next = C_current - (g_val / g_prime_val)
        if abs(C_next - C_current) < 1e-6:
            return C_next
        C_current = C_next
    return C_current

# ==========================================
# 2. STRUKTUR SIMULASI KENDARAAN & PERSIMPANGAN
# ==========================================

# Konstanta Skala & Layar
SCREEN_W = 1000
SCREEN_H = 800
CX = SCREEN_W // 2
CY = SCREEN_H // 2
SCALE = 10 # 1 meter = 10 pixel

class Vehicle:
    def __init__(self, start_pos, direction_vec, origin, color):
        # position (x, y) dalam koordinat layar (pixel)
        self.x, self.y = start_pos
        # direction (dx, dy) = arah pergerakan (contoh: turun = 0, 1)
        self.dx, self.dy = direction_vec
        self.origin = origin
        self.color = color
        
        self.max_speed = random.uniform(80.0, 120.0) # Pixel per detik (8-12 m/s)
        self.current_speed = self.max_speed
        
        self.turn_choice = random.randint(0, 2) # 0=Lurus, 1=Kiri, 2=Kanan
        self.has_turned = False
        
        # Dimensi kendaraan
        if abs(self.dx) > 0.5:
            # Bergerak horizontal (Timur/Barat)
            self.width = 4.0 * SCALE
            self.height = 2.0 * SCALE
        else:
            # Bergerak vertikal (Utara/Selatan)
            self.width = 2.0 * SCALE
            self.height = 4.0 * SCALE

    def update_behavior(self, dt, current_phase, other_vehicles, is_yellow_phase):
        harus_berhenti = False
        
        is_my_phase = False
        if self.origin == "N" and current_phase == 0: is_my_phase = True
        if self.origin == "E" and current_phase == 1: is_my_phase = True
        if self.origin == "S" and current_phase == 2: is_my_phase = True
        if self.origin == "W" and current_phase == 3: is_my_phase = True

        at_stop_line = False
        # Utara bergerak turun (+y) -> Batas: 15-20 meter sebelum tengah (150-200 px sebelum CX)
        if self.origin == "N" and 200 < self.y < 250: at_stop_line = True
        # Selatan bergerak naik (-y) -> Batas: 15-20 meter sesudah tengah
        if self.origin == "S" and 550 < self.y < 600: at_stop_line = True
        # Timur bergerak kiri (-x)
        if self.origin == "E" and 650 < self.x < 700: at_stop_line = True
        # Barat bergerak kanan (+x)
        if self.origin == "W" and 300 < self.x < 350: at_stop_line = True

        # Jika di garis merah, wajib berhenti
        if at_stop_line and not is_my_phase:
            harus_berhenti = True
            # Belok kiri boleh menerobos (Kiri di ID = Belok Kiri Jalan Terus)
            if self.turn_choice == 1:
                ada_mobil = False
                for other in other_vehicles:
                    if other != self and other.origin != self.origin:
                        if abs(other.x - CX) < 150 and abs(other.y - CY) < 150:
                            if other.current_speed > 20.0:
                                ada_mobil = True
                                break
                if not ada_mobil: harus_berhenti = False

        # Rem saat lampu kuning jika masih jauh
        if is_my_phase and is_yellow_phase:
            far_enough = False
            # Jarak aman pengereman kuning: 18 - 35 meter (180 - 350 pixel)
            if self.origin == "N" and 50 < self.y < 220: far_enough = True
            if self.origin == "S" and 580 < self.y < 750: far_enough = True
            if self.origin == "E" and 680 < self.x < 850: far_enough = True
            if self.origin == "W" and 150 < self.x < 320: far_enough = True
            if far_enough: harus_berhenti = True

        # Anti tabrakan cerdas
        for depan in other_vehicles:
            if depan != self:
                dist_x = depan.x - self.x
                dist_y = depan.y - self.y
                dist = math.hypot(dist_x, dist_y)
                
                # Cek jarak jika kurang dari 7 meter (70 pixel)
                if 1.0 < dist < 70.0:
                    dot = (self.dx * (dist_x/dist)) + (self.dy * (dist_y/dist))
                    if dot > 0.8: # Mobil persis di depannya
                        harus_berhenti = True

        # Logika membelok
        if not self.has_turned:
            trigger_turn = False
            if self.turn_choice == 1: # Belok Kiri (Memotong ke lajur dekat)
                if self.origin == "N" and self.y >= CY + 40: self.dx, self.dy = -1, 0; self.y = CY + 40; trigger_turn = True
                if self.origin == "S" and self.y <= CY - 40: self.dx, self.dy = 1, 0; self.y = CY - 40; trigger_turn = True
                if self.origin == "E" and self.x <= CX - 40: self.dx, self.dy = 0, -1; self.x = CX - 40; trigger_turn = True
                if self.origin == "W" and self.x >= CX + 40: self.dx, self.dy = 0, 1; self.x = CX + 40; trigger_turn = True
            elif self.turn_choice == 2: # Belok Kanan
                if self.origin == "N" and self.y >= CY - 40: self.dx, self.dy = 1, 0; self.y = CY - 40; trigger_turn = True
                if self.origin == "S" and self.y <= CY + 40: self.dx, self.dy = -1, 0; self.y = CY + 40; trigger_turn = True
                if self.origin == "E" and self.x <= CX + 40: self.dx, self.dy = 0, 1; self.x = CX + 40; trigger_turn = True
                if self.origin == "W" and self.x >= CX - 40: self.dx, self.dy = 0, -1; self.x = CX - 40; trigger_turn = True

            if trigger_turn:
                self.has_turned = True
                if abs(self.dx) > 0.5:
                    self.width, self.height = 40, 20
                else:
                    self.width, self.height = 20, 40

        if harus_berhenti:
            self.current_speed -= 400.0 * dt
            if self.current_speed < 0: self.current_speed = 0
        else:
            self.current_speed += 150.0 * dt
            if self.current_speed > self.max_speed: self.current_speed = self.max_speed
            
            # Pelan saat berbelok di persimpangan
            if abs(self.x - CX) < 150 and abs(self.y - CY) < 150:
                if self.current_speed > 80.0: self.current_speed = 80.0

        self.x += self.dx * self.current_speed * dt
        self.y += self.dy * self.current_speed * dt

    def draw(self, surface):
        rect = pygame.Rect(0, 0, int(self.width), int(self.height))
        rect.center = (int(self.x), int(self.y))
        pygame.draw.rect(surface, self.color, rect)
        pygame.draw.rect(surface, (0,0,0), rect, 1) # Wireframe hitam
        
        # Lampu sein kuning/oranye
        if not self.has_turned and abs(self.x - CX) < 250 and abs(self.y - CY) < 250:
            sein_rect = pygame.Rect(0, 0, 10, 10)
            sein_rect.center = (int(self.x), int(self.y))
            if self.turn_choice == 1:
                pygame.draw.rect(surface, (255,255,0), sein_rect) # Kiri
            elif self.turn_choice == 2:
                pygame.draw.rect(surface, (255,165,0), sein_rect) # Kanan


# ==========================================
# 3. KELAS PENGATUR PERSIMPANGAN
# ==========================================
class Intersection:
    def __init__(self, cycle_time):
        self.optimal_cycle = cycle_time
        self.vehicles = []
        self.timer_lampu = 0.0
        self.timer_spawn = 0.0
        self.current_phase = 0
        self.is_yellow_phase = False
        self.font = pygame.font.SysFont("Arial", 28, bold=True)

    def spawn(self, dt, interval_ns, interval_ew):
        self.timer_spawn += dt
        avg_interval = (interval_ns + interval_ew) / 2.0
        
        if self.timer_spawn > avg_interval:
            self.timer_spawn = 0.0
            
            # Batasan 6 kendaraan per antrian (Mencegah tumpukan)
            count_n, count_e, count_s, count_w = 0, 0, 0, 0
            for v in self.vehicles:
                if v.origin == "N": count_n += 1
                elif v.origin == "E": count_e += 1
                elif v.origin == "S": count_s += 1
                elif v.origin == "W": count_w += 1
                
            r = random.randint(0, 3)
            c = (random.randint(55,255), random.randint(55,255), random.randint(55,255))
            
            # Spawn dengan limit
            if r == 0 and count_n < 6:
                self.vehicles.append(Vehicle((CX - 40, -50), (0, 1), "N", c))
            elif r == 1 and count_e < 6:
                self.vehicles.append(Vehicle((SCREEN_W + 50, CY - 40), (-1, 0), "E", c))
            elif r == 2 and count_s < 6:
                self.vehicles.append(Vehicle((CX + 40, SCREEN_H + 50), (0, -1), "S", c))
            elif r == 3 and count_w < 6:
                self.vehicles.append(Vehicle((-50, CY + 40), (1, 0), "W", c))

    def update(self, dt, interval_ns, interval_ew):
        self.timer_lampu += dt
        phase_duration = self.optimal_cycle / 4.0
        actual_yellow = min(2.0, phase_duration * 0.25)
        
        # Pengatur transisi Lampu Lalu Lintas
        if not self.is_yellow_phase:
            if self.timer_lampu > (phase_duration - actual_yellow):
                self.is_yellow_phase = True
        else:
            if self.timer_lampu > phase_duration:
                self.timer_lampu = 0.0
                self.is_yellow_phase = False
                self.current_phase = (self.current_phase + 1) % 4
                
        self.spawn(dt, interval_ns, interval_ew)
        
        for v in self.vehicles:
            v.update_behavior(dt, self.current_phase, self.vehicles, self.is_yellow_phase)
            
        # Hapus mobil jauh
        self.vehicles = [v for v in self.vehicles if -100 <= v.x <= SCREEN_W+100 and -100 <= v.y <= SCREEN_H+100]

    def draw(self, surface):
        # 1. Gambar Jalan Aspal
        pygame.draw.rect(surface, (50,50,50), (CX - 80, 0, 160, SCREEN_H)) # NS
        pygame.draw.rect(surface, (50,50,50), (0, CY - 80, SCREEN_W, 160)) # EW
        
        # 2. Garis Marka Putus-Putus
        def draw_dashed_line(surf, color, start_pos, end_pos, width=2, dash_length=15):
            x1, y1 = start_pos
            x2, y2 = end_pos
            dl = math.hypot(x2 - x1, y2 - y1)
            if dl == 0: return
            dx, dy = (x2 - x1) / dl, (y2 - y1) / dl
            dash_count = int(dl / dash_length)
            for i in range(dash_count):
                if i % 2 == 0:
                    start = (x1 + i * dash_length * dx, y1 + i * dash_length * dy)
                    end = (x1 + (i + 1) * dash_length * dx, y1 + (i + 1) * dash_length * dy)
                    pygame.draw.line(surf, color, start, end, width)

        draw_dashed_line(surface, (255,255,255), (CX, 0), (CX, CY - 140), 4) # Marka Utara
        draw_dashed_line(surface, (255,255,255), (CX, CY + 140), (CX, SCREEN_H), 4) # Marka Selatan
        draw_dashed_line(surface, (255,255,255), (0, CY), (CX - 140, CY), 4) # Marka Barat
        draw_dashed_line(surface, (255,255,255), (CX + 140, CY), (SCREEN_W, CY), 4) # Marka Timur

        # 3. Zebra Cross
        for i in range(-70, 80, 20):
            if abs(i) < 10: continue
            pygame.draw.rect(surface, (255,255,255), (CX + i - 5, CY - 130, 10, 20)) # Utara
            pygame.draw.rect(surface, (255,255,255), (CX + i - 5, CY + 110, 10, 20)) # Selatan
            pygame.draw.rect(surface, (255,255,255), (CX + 110, CY + i - 5, 20, 10)) # Timur
            pygame.draw.rect(surface, (255,255,255), (CX - 130, CY + i - 5, 20, 10)) # Barat

        # 4. Stop Line (Belakang Zebra Cross)
        pygame.draw.rect(surface, (255,255,255), (CX - 75, CY - 145, 75, 5)) # Utara
        pygame.draw.rect(surface, (255,255,255), (CX, CY + 140, 75, 5)) # Selatan
        pygame.draw.rect(surface, (255,255,255), (CX + 140, CY - 75, 5, 75)) # Timur
        pygame.draw.rect(surface, (255,255,255), (CX - 145, CY, 5, 75)) # Barat

        # 5. Gambar Kendaraan
        for v in self.vehicles:
            v.draw(surface)

        # 6. Gambar Tiang Lampu & Countdown
        poles = [
            (CX - 100, CY - 150), # N
            (CX + 150, CY - 100), # E
            (CX + 100, CY + 150), # S
            (CX - 150, CY + 100)  # W
        ]
        
        phase_duration = self.optimal_cycle / 4.0
        
        for i, pos in enumerate(poles):
            pygame.draw.rect(surface, (20,20,20), (pos[0]-15, pos[1]-15, 30, 30)) # Housing Hitam
            
            c_top, c_mid, c_bot = (100,0,0), (100,100,0), (0,100,0) # Warna Redup (Mati)
            
            if self.current_phase == i:
                if self.is_yellow_phase: c_mid = (255,255,0)
                else: c_bot = (0,255,0)
            else:
                c_top = (255,0,0)
                
            # Gambar 3 lingkaran kecil warna warni di dalam housing
            pygame.draw.circle(surface, c_top, (pos[0]-8, pos[1]), 4)
            pygame.draw.circle(surface, c_mid, (pos[0], pos[1]), 4)
            pygame.draw.circle(surface, c_bot, (pos[0]+8, pos[1]), 4)
            
            # Render Teks Hitung Mundur
            status_text = ""
            text_color = (255,255,255)
            
            if self.current_phase == i:
                if self.is_yellow_phase:
                    sisa = math.ceil(phase_duration - self.timer_lampu)
                    status_text = f"{sisa}"
                    text_color = (255,255,0)
                else:
                    actual_yellow = min(2.0, phase_duration * 0.25)
                    sisa = math.ceil((phase_duration - actual_yellow) - self.timer_lampu)
                    status_text = f"{sisa}"
                    text_color = (0,255,0)
            else:
                fase_tunggu = (i - self.current_phase + 4) % 4
                waktu_tunggu = (fase_tunggu * phase_duration) - self.timer_lampu
                sisa = math.ceil(waktu_tunggu)
                status_text = f"{sisa}"
                text_color = (255,0,0)
                
            text_surf = self.font.render(status_text, True, text_color)
            shadow_surf = self.font.render(status_text, True, (0,0,0))
            
            surface.blit(shadow_surf, (pos[0]-10 + 2, pos[1]-40 + 2))
            surface.blit(text_surf, (pos[0]-10, pos[1]-40))

# ==========================================
# 4. LOOP UTAMA & MANAJEMEN STATE
# ==========================================
def recalculate(is_sibuk):
    Matriks_A = [[4, -1, 0, -1], [-1, 4, -1, 0], [0, -1, 4, -1], [-1, 0, -1, 3]]
    if is_sibuk:
        Vektor_B = [120, 90, 150, 110]
    else:
        Vektor_B = [30, 20, 40, 25]
        
    L, U = lu_decomposition(Matriks_A)
    flow = solve_lu(L, U, Vektor_B)
    
    sum_flow = sum(flow)
    param_a = sum_flow * 0.05
    param_b = sum_flow * 150.0
    siklus_mat = newton_raphson(60.0, param_a, param_b)
    
    if is_sibuk:
        interval_ns = (60.0 / ((flow[0] + flow[2]) / 2)) * 0.5
        interval_ew = (60.0 / ((flow[1] + flow[3]) / 2)) * 0.5
    else:
        interval_ns = (60.0 / ((flow[0] + flow[2]) / 2)) * 4.0
        interval_ew = (60.0 / ((flow[1] + flow[3]) / 2)) * 4.0
        
    siklus_vis = max(10.0, siklus_mat / 5.0)
    return interval_ns, interval_ew, siklus_vis, siklus_mat

def main():
    pygame.init()
    screen = pygame.display.set_mode((SCREEN_W, SCREEN_H))
    pygame.display.set_caption("Simulasi Lalu Lintas - Pygame 2D (Optimasi Numerik)")
    clock = pygame.time.Clock()
    
    is_jam_sibuk = False
    int_ns, int_ew, sik_vis, sik_mat = recalculate(is_jam_sibuk)
    
    dunia = Intersection(sik_vis)
    
    sim_hours = 6
    sim_mins = 0.0
    
    ui_font = pygame.font.SysFont("Arial", 22, bold=True)
    small_font = pygame.font.SysFont("Arial", 16)
    
    running = True
    while running:
        dt = clock.tick(60) / 1000.0 # DT dalam detik (Max 60 FPS)
        
        # Simulasi Waktu: 1 Detik Nyata = 2 Menit Simulasi
        sim_mins += dt * 2.0
        
        # Event Keyboard
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            if event.type == pygame.KEYDOWN:
                if event.key == pygame.K_RIGHT:
                    sim_mins += 60.0 # Loncat 1 jam secara manual
                    
        # Evaluasi Waktu 24H
        if sim_mins >= 60.0:
            sim_mins -= 60.0
            sim_hours += 1
            if sim_hours >= 24: sim_hours = 0
            
            now_sibuk = False
            if (7 <= sim_hours < 9) or (11 <= sim_hours < 13) or (16 <= sim_hours < 18):
                now_sibuk = True
                
            if now_sibuk != is_jam_sibuk:
                is_jam_sibuk = now_sibuk
                int_ns, int_ew, sik_vis, sik_mat = recalculate(is_jam_sibuk)
                dunia.optimal_cycle = sik_vis

        # Render & Update
        screen.fill((160, 200, 160)) # Warna rumput pucat
        
        dunia.update(dt, int_ns, int_ew)
        dunia.draw(screen)
        
        # HUD Panel
        panel_rect = pygame.Rect(10, 10, 400, 120)
        pygame.draw.rect(screen, (255,255,255), panel_rect)
        pygame.draw.rect(screen, (0,0,0), panel_rect, 2)
        
        # Teks Informasi
        time_text = ui_font.render(f"Jam Simulasi: {sim_hours:02d}:{int(sim_mins):02d}", True, (0,100,0))
        status_color = (255,0,0) if is_jam_sibuk else (0,0,255)
        status_text = ui_font.render("Status: " + ("JAM SIBUK" if is_jam_sibuk else "JAM SEPI"), True, status_color)
        siklus_text = small_font.render(f"Siklus Optimum Kalkulasi: {sik_mat:.2f} dtk", True, (0,0,0))
        help_text = small_font.render("Tekan [PANAH KANAN] untuk maju 1 Jam", True, (100,100,100))
        
        # Gambar teks ke layar
        screen.blit(time_text, (20, 20))
        screen.blit(status_text, (20, 50))
        screen.blit(siklus_text, (20, 80))
        screen.blit(help_text, (20, 100))
        
        pygame.display.flip()

    pygame.quit()
    sys.exit()

if __name__ == "__main__":
    main()
