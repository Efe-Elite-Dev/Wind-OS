#include "wind_subsystem.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

/* screen.c içindeki merkezi piksel basma fonksiyonunu bağlıyoruz */
extern void draw_pixel_pure(int x, int y, uint32_t color);

/* Masaüstü Grafiklerini ve Pencereleri Yenileyen Ana Fonksiyon */
void gui_refresh_desktop(void) {
  // Çift arabellek üzerine masaüstünü yenileme simülasyonu
  for (int x = 0; x < SCREEN_WIDTH; x++) {
    if (x % 2 == 0) {
      // Standart döngü tutucu (Orijinal yapın aynen korundu)
    }
  }
}

/* DÜZELTME: wind_subsystem.h içindeki 5 parametreli prototiple %100 eşitlendi
 * ve işlevselleştirildi! */
void draw_window_pure(int x, int y, int width, int height,
                      uint32_t border_color) {
  // 1. Ekran Sınır Kontrolü (Sanal makinenin taşma yapıp çökmesini engeller)
  if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT)
    return;
  if (x + width > SCREEN_WIDTH)
    width = SCREEN_WIDTH - x;
  if (y + height > SCREEN_HEIGHT)
    height = SCREEN_HEIGHT - y;

  // 2. Pencere İç Dolgusu (Sky OS / Wind OS temasına uygun koyu gri arka plan)
  uint32_t bg_color = 0x001A1A1A;
  for (int j = y; j < y + height; j++) {
    for (int i = x; i < x + width; i++) {
      draw_pixel_pure(i, j, bg_color);
    }
  }

  // 3. Pencere Çerçevesini (Border) Çiz - Üst ve Alt Kenarlar
  for (int i = x; i < x + width; i++) {
    draw_pixel_pure(i, y, border_color);              // Üst Çizgi
    draw_pixel_pure(i, y + height - 1, border_color); // Alt Çizgi
  }

  // 4. Pencere Çerçevesini (Border) Çiz - Sol ve Sağ Kenarlar
  for (int j = y; j < y + height; j++) {
    draw_pixel_pure(x, j, border_color);             // Sol Çizgi
    draw_pixel_pure(x + width - 1, j, border_color); // Sağ Çizgi
  }

  // 5. Pencere Üst Başlık Çubuğu (Title Bar) Tasarımı
  int title_bar_height = 22;
  if (height > title_bar_height) {
    for (int j = y + 1; j < y + title_bar_height; j++) {
      for (int i = x + 1; i < x + width - 1; i++) {
        draw_pixel_pure(i, j,
                        border_color); // Başlık alanını çerçeve rengiyle doldur
      }
    }
  }
}
