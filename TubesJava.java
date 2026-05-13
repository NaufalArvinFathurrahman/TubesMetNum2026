import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.Random;

public class TubesJava extends JPanel implements ActionListener, KeyListener {

    // Konstanta Skala
    public static final int SCREEN_W = 1000;
    public static final int SCREEN_H = 800;
    public static final int CX = SCREEN_W / 2;
    public static final int CY = SCREEN_H / 2;
    public static final int SCALE = 10;
    
    // ==========================================
    // 1. MESIN MATEMATIKA (Dekomposisi LU & Newton-Raphson)
    // ==========================================
    static class MathEngine {
        // Pemecah Sistem Persamaan Linear (SPL) Langsung dengan Dekomposisi LU
        public static double[] solve_lu_direct(double[][] A, double[] B) {
            int n = A.length;
            double[][] L = new double[n][n];
            double[][] U = new double[n][n];
            
            // Dekomposisi LU
            for (int i = 0; i < n; i++) {
                for (int k = i; k < n; k++) {
                    double sum_upper = 0;
                    for (int j = 0; j < i; j++) sum_upper += L[i][j] * U[j][k];
                    U[i][k] = A[i][k] - sum_upper;
                }
                for (int k = i; k < n; k++) {
                    if (i == k) {
                        L[i][i] = 1.0;
                    } else {
                        double sum_lower = 0;
                        for (int j = 0; j < i; j++) sum_lower += L[k][j] * U[j][i];
                        L[k][i] = (A[k][i] - sum_lower) / U[i][i];
                    }
                }
            }
            
            // Substitusi Maju - Mundur
            double[] Y = new double[n];
            double[] X = new double[n];
            
            for (int i = 0; i < n; i++) {
                double sum_y = 0;
                for (int j = 0; j < i; j++) sum_y += L[i][j] * Y[j];
                Y[i] = B[i] - sum_y;
            }
            
            for (int i = n - 1; i >= 0; i--) {
                double sum_x = 0;
                for (int j = i + 1; j < n; j++) sum_x += U[i][j] * X[j];
                X[i] = (Y[i] - sum_x) / U[i][i];
            }
            return X;
        }

        // Optimasi Non-Linear
        public static double newton_raphson(double initial_guess, double a, double b) {
            double C_current = initial_guess;
            for (int i = 0; i < 100; i++) {
                double g_val = a - (b / (C_current * C_current));
                double g_prime_val = 2 * b / (C_current * C_current * C_current);
                if (Math.abs(g_prime_val) < 1e-10) break;
                
                double C_next = C_current - (g_val / g_prime_val);
                if (Math.abs(C_next - C_current) < 1e-6) return C_next;
                C_current = C_next;
            }
            return C_current;
        }
    }

    // ==========================================
    // 2. KELAS KENDARAAN (Vehicle)
    // ==========================================
    static class Vehicle {
        double x, y;
        double dx, dy;
        String origin;
        Color color;
        double max_speed;
        double current_speed;
        int turn_choice;
        boolean has_turned;
        double width, height;
        static Random rand = new Random();

        public Vehicle(double startX, double startY, double dx, double dy, String origin, Color color) {
            this.x = startX;
            this.y = startY;
            this.dx = dx;
            this.dy = dy;
            this.origin = origin;
            this.color = color;
            this.max_speed = 80.0 + rand.nextDouble() * 40.0;
            this.current_speed = this.max_speed;
            this.turn_choice = rand.nextInt(3); // 0 = lurus, 1 = kiri, 2 = kanan
            this.has_turned = false;

            if (Math.abs(this.dx) > 0.5) {
                this.width = 4.0 * SCALE;
                this.height = 2.0 * SCALE;
            } else {
                this.width = 2.0 * SCALE;
                this.height = 4.0 * SCALE;
            }
        }

        public void updateBehavior(double dt, int current_phase, ArrayList<Vehicle> other_vehicles, boolean is_yellow_phase) {
            boolean harus_berhenti = false;
            boolean is_my_phase = false;
            
            if (origin.equals("N") && current_phase == 0) is_my_phase = true;
            if (origin.equals("E") && current_phase == 1) is_my_phase = true;
            if (origin.equals("S") && current_phase == 2) is_my_phase = true;
            if (origin.equals("W") && current_phase == 3) is_my_phase = true;

            // Stop line check
            boolean at_stop_line = false;
            if (origin.equals("N") && y > 200 && y < 250) at_stop_line = true;
            if (origin.equals("S") && y > 550 && y < 600) at_stop_line = true;
            if (origin.equals("E") && x > 650 && x < 700) at_stop_line = true;
            if (origin.equals("W") && x > 300 && x < 350) at_stop_line = true;

            // Berhenti lampu merah
            if (at_stop_line && !is_my_phase) {
                harus_berhenti = true;
                // Aturan belok kiri jalan terus
                if (turn_choice == 1) {
                    boolean ada_mobil = false;
                    for (Vehicle other : other_vehicles) {
                        if (other != this && !other.origin.equals(this.origin)) {
                            if (Math.abs(other.x - CX) < 150 && Math.abs(other.y - CY) < 150) {
                                if (other.current_speed > 20.0) {
                                    ada_mobil = true;
                                    break;
                                }
                            }
                        }
                    }
                    if (!ada_mobil) harus_berhenti = false;
                }
            }

            // Rem lampu kuning
            if (is_my_phase && is_yellow_phase) {
                boolean far_enough = false;
                if (origin.equals("N") && y > 50 && y < 220) far_enough = true;
                if (origin.equals("S") && y > 580 && y < 750) far_enough = true;
                if (origin.equals("E") && x > 680 && x < 850) far_enough = true;
                if (origin.equals("W") && x > 150 && x < 320) far_enough = true;
                if (far_enough) harus_berhenti = true;
            }

            // Anti tabrakan
            for (Vehicle depan : other_vehicles) {
                if (depan != this) {
                    double dist_x = depan.x - this.x;
                    double dist_y = depan.y - this.y;
                    double dist = Math.hypot(dist_x, dist_y);

                    if (dist > 1.0 && dist < 70.0) {
                        double dot = (this.dx * (dist_x / dist)) + (this.dy * (dist_y / dist));
                        if (dot > 0.8) harus_berhenti = true; // Jika mobil tepat di depan moncong
                    }
                }
            }

            // Logika belok
            if (!has_turned) {
                boolean trigger_turn = false;
                if (turn_choice == 1) { // Kiri
                    if (origin.equals("N") && y >= CY + 40) { dx = -1; dy = 0; y = CY + 40; trigger_turn = true; }
                    if (origin.equals("S") && y <= CY - 40) { dx = 1; dy = 0; y = CY - 40; trigger_turn = true; }
                    if (origin.equals("E") && x <= CX - 40) { dx = 0; dy = -1; x = CX - 40; trigger_turn = true; }
                    if (origin.equals("W") && x >= CX + 40) { dx = 0; dy = 1; x = CX + 40; trigger_turn = true; }
                } else if (turn_choice == 2) { // Kanan
                    if (origin.equals("N") && y >= CY - 40) { dx = 1; dy = 0; y = CY - 40; trigger_turn = true; }
                    if (origin.equals("S") && y <= CY + 40) { dx = -1; dy = 0; y = CY + 40; trigger_turn = true; }
                    if (origin.equals("E") && x <= CX + 40) { dx = 0; dy = 1; x = CX + 40; trigger_turn = true; }
                    if (origin.equals("W") && x >= CX - 40) { dx = 0; dy = -1; x = CX - 40; trigger_turn = true; }
                }

                if (trigger_turn) {
                    has_turned = true;
                    if (Math.abs(dx) > 0.5) { width = 40; height = 20; } 
                    else { width = 20; height = 40; }
                }
            }

            // Dinamika
            if (harus_berhenti) {
                current_speed -= 400.0 * dt;
                if (current_speed < 0) current_speed = 0;
            } else {
                current_speed += 150.0 * dt;
                if (current_speed > max_speed) current_speed = max_speed;
                // Pelan saat berbelok
                if (Math.abs(x - CX) < 150 && Math.abs(y - CY) < 150) {
                    if (current_speed > 80.0) current_speed = 80.0;
                }
            }

            x += dx * current_speed * dt;
            y += dy * current_speed * dt;
        }

        public void draw(Graphics2D g2d) {
            int drawX = (int) (x - width / 2);
            int drawY = (int) (y - height / 2);
            
            g2d.setColor(color);
            g2d.fillRect(drawX, drawY, (int)width, (int)height);
            g2d.setColor(Color.BLACK);
            g2d.drawRect(drawX, drawY, (int)width, (int)height);

            // Lampu Sein
            if (!has_turned && Math.abs(x - CX) < 250 && Math.abs(y - CY) < 250) {
                int seinX = (int) (x - 5);
                int seinY = (int) (y - 5);
                if (turn_choice == 1) {
                    g2d.setColor(Color.YELLOW);
                    g2d.fillRect(seinX, seinY, 10, 10);
                } else if (turn_choice == 2) {
                    g2d.setColor(Color.ORANGE);
                    g2d.fillRect(seinX, seinY, 10, 10);
                }
            }
        }
    }

    // ==========================================
    // 3. PENGATUR PERSIMPANGAN (Intersection)
    // ==========================================
    static class Intersection {
        double optimal_cycle;
        ArrayList<Vehicle> vehicles = new ArrayList<>();
        double timer_lampu = 0.0;
        double timer_spawn = 0.0;
        int current_phase = 0;
        boolean is_yellow_phase = false;
        Random rand = new Random();

        public Intersection(double cycle_time) {
            this.optimal_cycle = cycle_time;
        }

        public void spawn(double dt, double interval_ns, double interval_ew) {
            timer_spawn += dt;
            double avg_interval = (interval_ns + interval_ew) / 2.0;

            if (timer_spawn > avg_interval) {
                timer_spawn = 0.0;

                int count_n = 0, count_e = 0, count_s = 0, count_w = 0;
                for (Vehicle v : vehicles) {
                    if (v.origin.equals("N")) count_n++;
                    else if (v.origin.equals("E")) count_e++;
                    else if (v.origin.equals("S")) count_s++;
                    else if (v.origin.equals("W")) count_w++;
                }

                int r = rand.nextInt(4);
                Color c = new Color(rand.nextInt(200) + 55, rand.nextInt(200) + 55, rand.nextInt(200) + 55);

                if (r == 0 && count_n < 6) vehicles.add(new Vehicle(CX - 40, -50, 0, 1, "N", c));
                else if (r == 1 && count_e < 6) vehicles.add(new Vehicle(SCREEN_W + 50, CY - 40, -1, 0, "E", c));
                else if (r == 2 && count_s < 6) vehicles.add(new Vehicle(CX + 40, SCREEN_H + 50, 0, -1, "S", c));
                else if (r == 3 && count_w < 6) vehicles.add(new Vehicle(-50, CY + 40, 1, 0, "W", c));
            }
        }

        public void update(double dt, double interval_ns, double interval_ew) {
            timer_lampu += dt;
            double phase_duration = optimal_cycle / 4.0;
            double actual_yellow = Math.min(2.0, phase_duration * 0.25);

            if (!is_yellow_phase) {
                if (timer_lampu > (phase_duration - actual_yellow)) {
                    is_yellow_phase = true;
                }
            } else {
                if (timer_lampu > phase_duration) {
                    timer_lampu = 0.0;
                    is_yellow_phase = false;
                    current_phase = (current_phase + 1) % 4;
                }
            }

            spawn(dt, interval_ns, interval_ew);

            Iterator<Vehicle> it = vehicles.iterator();
            while (it.hasNext()) {
                Vehicle v = it.next();
                v.updateBehavior(dt, current_phase, vehicles, is_yellow_phase);
                if (v.x < -100 || v.x > SCREEN_W + 100 || v.y < -100 || v.y > SCREEN_H + 100) {
                    it.remove();
                }
            }
        }

        public void drawDashedLine(Graphics2D g2d, int x1, int y1, int x2, int y2) {
            Stroke dashed = new BasicStroke(3, BasicStroke.CAP_BUTT, BasicStroke.JOIN_BEVEL, 0, new float[]{15}, 0);
            g2d.setStroke(dashed);
            g2d.drawLine(x1, y1, x2, y2);
            g2d.setStroke(new BasicStroke(1)); // Reset back to normal
        }

        public void draw(Graphics2D g2d) {
            // Aspal Jalan
            g2d.setColor(new Color(50, 50, 50));
            g2d.fillRect(CX - 80, 0, 160, SCREEN_H);
            g2d.fillRect(0, CY - 80, SCREEN_W, 160);

            // Garis Marka
            g2d.setColor(Color.WHITE);
            drawDashedLine(g2d, CX, 0, CX, CY - 140);
            drawDashedLine(g2d, CX, CY + 140, CX, SCREEN_H);
            drawDashedLine(g2d, 0, CY, CX - 140, CY);
            drawDashedLine(g2d, CX + 140, CY, SCREEN_W, CY);

            // Zebra Cross
            for (int i = -70; i <= 70; i += 20) {
                if (Math.abs(i) < 10) continue;
                g2d.fillRect(CX + i - 5, CY - 130, 10, 20); // Utara
                g2d.fillRect(CX + i - 5, CY + 110, 10, 20); // Selatan
                g2d.fillRect(CX + 110, CY + i - 5, 20, 10); // Timur
                g2d.fillRect(CX - 130, CY + i - 5, 20, 10); // Barat
            }

            // Stop Line
            g2d.fillRect(CX - 75, CY - 145, 75, 5);
            g2d.fillRect(CX, CY + 140, 75, 5);
            g2d.fillRect(CX + 140, CY - 75, 5, 75);
            g2d.fillRect(CX - 145, CY, 5, 75);

            // Mobil
            for (Vehicle v : vehicles) v.draw(g2d);

            // Tiang Lampu
            int[][] poles = {
                {CX - 100, CY - 150}, {CX + 150, CY - 100},
                {CX + 100, CY + 150}, {CX - 150, CY + 100}
            };
            double phase_duration = optimal_cycle / 4.0;

            for (int i = 0; i < poles.length; i++) {
                int px = poles[i][0];
                int py = poles[i][1];
                g2d.setColor(new Color(20, 20, 20));
                g2d.fillRect(px - 15, py - 15, 30, 30); // Housing Hitam

                Color cTop = new Color(100, 0, 0);
                Color cMid = new Color(100, 100, 0);
                Color cBot = new Color(0, 100, 0);

                if (current_phase == i) {
                    if (is_yellow_phase) cMid = Color.YELLOW;
                    else cBot = Color.GREEN;
                } else {
                    cTop = Color.RED;
                }

                g2d.setColor(cTop); g2d.fillOval(px - 8 - 4, py - 4, 8, 8);
                g2d.setColor(cMid); g2d.fillOval(px - 4, py - 4, 8, 8);
                g2d.setColor(cBot); g2d.fillOval(px + 8 - 4, py - 4, 8, 8);

                // UI Countdown
                String status_text = "";
                Color text_color = Color.WHITE;

                if (current_phase == i) {
                    if (is_yellow_phase) {
                        int sisa = (int) Math.ceil(phase_duration - timer_lampu);
                        status_text = String.valueOf(sisa);
                        text_color = Color.YELLOW;
                    } else {
                        double actual_yellow = Math.min(2.0, phase_duration * 0.25);
                        int sisa = (int) Math.ceil((phase_duration - actual_yellow) - timer_lampu);
                        status_text = String.valueOf(sisa);
                        text_color = Color.GREEN;
                    }
                } else {
                    int fase_tunggu = (i - current_phase + 4) % 4;
                    double waktu_tunggu = (fase_tunggu * phase_duration) - timer_lampu;
                    int sisa = (int) Math.ceil(waktu_tunggu);
                    status_text = String.valueOf(sisa);
                    text_color = Color.RED;
                }

                g2d.setFont(new Font("Arial", Font.BOLD, 28));
                g2d.setColor(Color.BLACK);
                g2d.drawString(status_text, px - 10 + 2, py - 40 + 2); // Shadow
                g2d.setColor(text_color);
                g2d.drawString(status_text, px - 10, py - 40);         // Text Utama
            }
        }
    }

    // ==========================================
    // 4. MAIN GAME LOOP & MANAJEMEN WINDOW
    // ==========================================
    private Timer timer;
    private Intersection dunia;
    private boolean is_jam_sibuk = false;
    private double interval_ns, interval_ew, siklus_vis, siklus_mat;
    private int sim_hours = 6;
    private double sim_mins = 0.0;
    private long lastTime;

    public TubesJava() {
        setPreferredSize(new Dimension(SCREEN_W, SCREEN_H));
        setBackground(new Color(160, 200, 160)); // Warna rumput
        setFocusable(true);
        addKeyListener(this);

        recalculate(is_jam_sibuk);
        dunia = new Intersection(siklus_vis);
        lastTime = System.currentTimeMillis();

        timer = new Timer(16, this); // Delay 16ms = ~60 FPS
        timer.start();
    }

    private void recalculate(boolean sibuk) {
        double[][] Matriks_A = {
            { 4, -1,  0, -1}, {-1,  4, -1,  0}, { 0, -1,  4, -1}, {-1,  0, -1,  3}
        };
        double[] Vektor_B;
        if (sibuk) {
            Vektor_B = new double[]{120, 90, 150, 110};
        } else {
            Vektor_B = new double[]{30, 20, 40, 25};
        }

        double[] flow = MathEngine.solve_lu_direct(Matriks_A, Vektor_B);

        double sum_flow = 0;
        for (double f : flow) sum_flow += f;

        double param_a = sum_flow * 0.05;
        double param_b = sum_flow * 150.0;
        siklus_mat = MathEngine.newton_raphson(60.0, param_a, param_b);

        if (sibuk) {
            interval_ns = (60.0 / ((flow[0] + flow[2]) / 2)) * 0.5;
            interval_ew = (60.0 / ((flow[1] + flow[3]) / 2)) * 0.5;
        } else {
            interval_ns = (60.0 / ((flow[0] + flow[2]) / 2)) * 4.0;
            interval_ew = (60.0 / ((flow[1] + flow[3]) / 2)) * 4.0;
        }

        siklus_vis = Math.max(10.0, siklus_mat / 5.0);
    }

    @Override
    public void actionPerformed(ActionEvent e) {
        long now = System.currentTimeMillis();
        double dt = (now - lastTime) / 1000.0;
        lastTime = now;

        // Simulasi Waktu: 1 Detik Nyata = 2 Menit Simulasi
        sim_mins += dt * 2.0;

        if (sim_mins >= 60.0) {
            sim_mins -= 60.0;
            sim_hours++;
            if (sim_hours >= 24) sim_hours = 0;

            // Logika jadwal jam sibuk
            boolean now_sibuk = false;
            if ((sim_hours >= 7 && sim_hours < 9) || 
                (sim_hours >= 11 && sim_hours < 13) || 
                (sim_hours >= 16 && sim_hours < 18)) {
                now_sibuk = true;
            }

            if (now_sibuk != is_jam_sibuk) {
                is_jam_sibuk = now_sibuk;
                recalculate(is_jam_sibuk);
                dunia.optimal_cycle = siklus_vis; // Update siklus secara dinamis di tengah jalan
            }
        }

        dunia.update(dt, interval_ns, interval_ew);
        repaint(); // Memanggil ulang paintComponent
    }

    @Override
    protected void paintComponent(Graphics g) {
        super.paintComponent(g);
        Graphics2D g2d = (Graphics2D) g;
        
        // Agar grafis halus (Anti-Aliasing)
        g2d.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);

        dunia.draw(g2d);

        // Panel Transparan HUD Kiri Atas
        g2d.setColor(new Color(255, 255, 255, 200));
        g2d.fillRect(10, 10, 400, 120);
        g2d.setColor(Color.BLACK);
        g2d.drawRect(10, 10, 400, 120);

        g2d.setFont(new Font("Arial", Font.BOLD, 22));
        g2d.setColor(new Color(0, 100, 0));
        g2d.drawString(String.format("Jam Simulasi: %02d:%02d", sim_hours, (int) sim_mins), 20, 35);

        if (is_jam_sibuk) g2d.setColor(Color.RED);
        else g2d.setColor(Color.BLUE);
        g2d.drawString("Status: " + (is_jam_sibuk ? "JAM SIBUK" : "JAM SEPI"), 20, 65);

        g2d.setFont(new Font("Arial", Font.PLAIN, 16));
        g2d.setColor(Color.BLACK);
        g2d.drawString(String.format("Siklus Optimum Kalkulasi: %.2f dtk", siklus_mat), 20, 95);

        g2d.setColor(Color.DARK_GRAY);
        g2d.drawString("Tekan [PANAH KANAN] untuk maju 1 Jam", 20, 115);
    }

    // --- Kontrol Keyboard ---
    @Override
    public void keyPressed(KeyEvent e) {
        if (e.getKeyCode() == KeyEvent.VK_RIGHT) {
            sim_mins += 60.0; // Lompat jam manual
        }
    }
    @Override public void keyReleased(KeyEvent e) {}
    @Override public void keyTyped(KeyEvent e) {}

    // Method pembuka jendela program
    public static void main(String[] args) {
        // Harus dijalankan pada thread khusus Swing
        SwingUtilities.invokeLater(() -> {
            JFrame frame = new JFrame("Simulasi Lalu Lintas - Java Swing (Optimasi Numerik)");
            TubesJava panel = new TubesJava();
            frame.add(panel);
            frame.pack();
            frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
            frame.setLocationRelativeTo(null); // Membuka jendela di tengah monitor
            frame.setResizable(false);
            frame.setVisible(true);
        });
    }
}
