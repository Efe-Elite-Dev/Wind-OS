#include "wind_subsystem.h"
#include "exe_subsystem.h" /* Kendi başlık dosyasını dahil ederek prototipleri kilitliyoruz */

/* Harici grafik dosyalarından fonksiyonları güvenli bir şekilde bağlıyoruz */
extern void draw_pixel_pure(int x, int y, uint32_t color);
extern void draw_window_pure(int x, int y, int width, int height, uint32_t border_color);

/* OOBE aşamalarını takip eden global değişkenin tanımı (Header'daki extern'i besler) */
int setup_stage = 0;

/* Özel pencereler ve arayüz bileşenleri için gövde doldurma fonksiyonu */
void draw_custom_window(int x, int y, int width, int height, const char* title, uint32_t body_color) {
    // Derleyicinin title uyarısını susturmak ve ileride kullanmak üzere sabitlemek
    (void)title;

    // Pencerenin içini (gövdesini) saf piksellerle doldur
    for (int i = 1; i < height; i++) {
        for (int j = 1; j < width; j++) {
            draw_pixel_pure(x + j, y + i, body_color);
        }
    }

    // Pencerenin dış çerçevesini wind_subsystem altındaki GUI motoruyla mühürle
    draw_window_pure(x, y, width, height, 0x00FFFFFF); // Saf beyaz dış hat
}

/* Rüzgar Alt Sistemi Çalıştırıcı ve Görev Yöneticisi */
void run_exe_subsystem(void) {
    // Masaüstü arka plan rengini ve yapay zeka pencerelerini besle
    // Koyu lacivert AI arayüz tonu: 0x000B1E36
    
    // Test amaçlı ilk sabit pencereleri ekrana basıyoruz
    draw_custom_window(50, 50, 300, 200, "Wind OS AI Monitor", 0x001A1A1A);
    draw_custom_window(400, 100, 250, 150, "Task Resource Predictor", 0x00222222);
}
