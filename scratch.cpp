#include <iostream>
using namespace std;
double hitung_delay_webster(double C, double* q, double s, double L) {
    double delay_total = 0;
    double Y = 0;
    for (int i=0; i<4; i++) Y += q[i]/s;
    if (C <= L) return 999999.0;
    for (int i = 0; i < 4; i++) {
        double y_i = q[i] / s;
        double g_i = (y_i / Y) * (C - L);
        double lambda = g_i / C;            
        double x = q[i] / (lambda * s);      
        if (x >= 0.99) return 999999.0;
        double pengali_1 = (1 - lambda);
        double pembilang_d1 = C * pengali_1 * pengali_1; 
        double penyebut_d1 = 2 * (1 - (lambda * x));
        double d1 = pembilang_d1 / penyebut_d1;
        double pembilang_d2 = x * x; 
        double penyebut_d2 = 2 * q[i] * (1 - x);
        double d2 = pembilang_d2 / penyebut_d2;
        delay_total += (d1 + d2);
    }
    return delay_total;
}
double turunan_pertama(double C, double* q, double s, double L) {
    double h = 0.001; 
    return (hitung_delay_webster(C + h, q, s, L) - hitung_delay_webster(C, q, s, L)) / h; 
}
double turunan_kedua(double C, double* q, double s, double L) {
    double h = 0.001;
    return (turunan_pertama(C + h, q, s, L) - turunan_pertama(C, q, s, L)) / h;
}
int main() {
    double q[4] = {0.54, 0.88, 0.82, 0.46};
    double C = 20.0;
    for(int i=0; i<10; i++) {
        double f1 = turunan_pertama(C, q, 20, 15);
        double f2 = turunan_kedua(C, q, 20, 15);
        C = C - f1/f2;
        cout << C << endl;
    }
}
