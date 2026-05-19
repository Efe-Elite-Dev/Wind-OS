#ifndef SKY_CORE_H
#define SKY_CORE_H

#include <stdint.h>

// Global Değişken (Herkesin göreceği isim bu olacak)
extern uint32_t* GRAPHICS_FRAMEBUFFER;
extern int SCREEN_W;
extern int SCREEN_H;

// Grafik Fonksiyonları
void draw_pixel(int x, int y, uint32_t color);
void draw_rect(int x, int y, int w, int h, uint32_t color);
void draw_rounded_rect(int x, int y, int w, int h, int radius, uint32_t color);
void render_ui(void);
void force_graphics_hardware(void);

#endif
