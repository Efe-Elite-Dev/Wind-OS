#include "gui.h"
#include "setup.h"
#include "screen.h" // Küresel draw_pixel_pure fonksiyonunu burası sağlar

void draw_rect_pure(int x, int y, int w, int h, uint8_t color) {
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            draw_pixel_pure(x + j, y + i, color);
        }
    }
}

void draw_window_pure(int x, int y, int w, int h, const char* title) {
    (void)title;
    draw_rect_pure(x, y, w, h, COLOR_WIND_CARD);

    for (int i = 0; i < w; i++) draw_pixel_pure(x + i, y, COLOR_WIND_3D_LIGHT);
    for (int i = 0; i < h; i++) draw_pixel_pure(x, y + i, COLOR_WIND_3D_LIGHT);
    for (int i = 0; i < w; i++) draw_pixel_pure(x + i, y + h - 1, COLOR_WIND_3D_DARK);
    for (int i = 0; i < h; i++) draw_pixel_pure(x + w - 1, y + i, COLOR_WIND_3D_DARK);

    draw_rect_pure(x + 2, y + 2, w - 4, 12, COLOR_WIND_PRIMARY);
}

void gui_init(void) {
    draw_rect_pure(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, COLOR_WIND_BG);
    draw_window_pure(40, 30, 240, 140, "Wind OS System");
}
