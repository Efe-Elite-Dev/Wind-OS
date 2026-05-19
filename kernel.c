#include "sky_core.h"

// Global değişkenlerin ASIL tanımları burada
uint32_t* FRAMEBUFFER = (uint32_t*)0xFD000000;
int SCREEN_W = 1024;
int SCREEN_H = 768;

void draw_pixel(int x, int y, uint32_t color) {
    if (FRAMEBUFFER && x >= 0 && x < SCREEN_W && y >= 0 && y < SCREEN_H) {
        FRAMEBUFFER[y * SCREEN_W + x] = color;
    }
}

void draw_rect(int x, int y, int w, int h, uint32_t color) {
    for (int i = y; i < y + h; i++) {
        for (int j = x; j < x + w; j++) draw_pixel(j, i, color);
    }
}

void draw_rounded_rect(int x, int y, int w, int h, int radius, uint32_t color) {
    for (int i = y; i < y + h; i++) {
        for (int j = x; j < x + w; j++) {
            if ((i < y + radius || i > y + h - radius - 1) && 
                (j < x + radius || j > x + w - radius - 1)) continue;
            draw_pixel(j, i, color);
        }
    }
}

void render_ui(void) {
    // Arka plan
    for (int i = 0; i < SCREEN_H; i++) {
        uint32_t color = (0xFF << 24) | (30 << 16) | (30 << 8) | 60;
        for (int j = 0; j < SCREEN_W; j++) draw_pixel(j, i, color);
    }
    // Kart
    draw_rounded_rect(200, 150, 624, 468, 20, 0xFFFFFFFF);
}

void kernel_main(void* mboot_ptr, uint32_t magic) {
    force_graphics_hardware();
    render_ui();
    while(1) { __asm__ volatile("hlt"); }
}
