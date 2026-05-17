#include "wind_subsystem.h"

/* Ekran Çözünürlük Standartları */
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

/* Harici VRAM İşaretçileri (kernel.c içindeki ana işaretçiyi besler) */
extern uint32_t *vbe_vram;
extern uint32_t vbe_pitch;

/* DÜZELTME: Linker'ın aradığı o meşhur arka bellek arabelleği (Double Buffer)
 * resmi tanımı! */
uint32_t back_buffer[SCREEN_WIDTH * SCREEN_HEIGHT];

/**
 * @brief Grafik modunu ve arka belleği (Back Buffer) ilk aşama için temizler.
 */
void init_graph_mode(void) {
  for (int i = 0; i < (SCREEN_WIDTH * SCREEN_HEIGHT); i++) {
    back_buffer[i] = 0x00000000; // Saf siyah ile ön hazırlık
  }
}

/**
 * @brief Arka belleğe (Back Buffer) pürüzsüz ve saf bir piksel basar.
 */
void draw_pixel_pure(int x, int y, uint32_t color) {
  if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
    back_buffer[y * SCREEN_WIDTH + x] = color;
  }
}

/**
 * @brief Tüm arka belleği tek bir renk ile kaplar.
 */
void clear_screen_gfx(uint32_t color) {
  for (int i = 0; i < (SCREEN_WIDTH * SCREEN_HEIGHT); i++) {
    back_buffer[i] = color;
  }
}

/**
 * @brief Arka bellekteki (Back Buffer) tüm veriyi tek seferde gerçek VRAM'e
 * üfler. Bu fonksiyon gui.c içindeki gui_refresh_desktop tarafından
 * çağrılacaktır.
 */
void screen_flush(void) {
  if (vbe_vram == 0)
    return;

  for (int y = 0; y < SCREEN_HEIGHT; y++) {
    for (int x = 0; x < SCREEN_WIDTH; x++) {
      vbe_vram[y * (vbe_pitch / 4) + x] = back_buffer[y * SCREEN_WIDTH + x];
    }
  }
}
