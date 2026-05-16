#include "wind_subsystem.h"

void draw_window_pure(int x, int y, int width, int height, uint32_t border_color) {
    // Üst ve Alt Çizgileri Piksel Piksel Çiz
    for (int i = 0; i < width; i++) {
        draw_pixel_pure(x + i, y, border_color);          // Üst kenar
        draw_pixel_pure(x + i, y + height, border_color); // Alt kenar
    }
    
    // Sağ ve Sol Çizgileri Piksel Piksel Çiz
    for (int i = 0; i < height; i++) {
        draw_pixel_pure(x, y + i, border_color);         // Sol kenar
        draw_pixel_pure(x + width, y + i, border_color); // Sağ kenar
    }
}
