#ifndef EXE_SUBSYSTEM_H
#define EXE_SUBSYSTEM_H

#include <stdint.h>

/* =============================================================================
   6. EXE SUBSYSTEM VE GRAFİK ÇİZİM MOTORU UZANTILARI
   ============================================================================= */

/* Grafik alt sisteminin ekrana pürüzsüz pikseller ve şekiller basmasını sağlayan motorlar */
void draw_pixel_pure(int x, int y, uint32_t color);
void draw_filled_rect(int x, int y, int width, int height, uint32_t color);
void draw_rect_outline(int x, int y, int width, int height, uint32_t color);
void draw_custom_window(int x, int y, int width, int height, const char* title, uint32_t body_color);

/* OOBE Aşamalarını takip eden global değişken (Dış dosyalardan erişim için) */
extern int setup_stage;

#endif /* EXE_SUBSYSTEM_H */
