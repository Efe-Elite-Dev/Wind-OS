#include "mouse.h"

// Standart kütüphane yerine kendi türlerimizi tanımlıyoruz
typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned short uint16_t;

// inb fonksiyonu donanım portundan veri okur
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static int mouse_x = 400, mouse_y = 300;

// Dışarıda tanımlı (gui.c içinde) olduğunu belirtiyoruz
extern void update_cursor_position(int x, int y);

void init_mouse() {
    // Mouse'u başlatma komutları buraya
}

void mouse_handler() {
    uint8_t status = inb(0x64);
    if (!(status & 0x01)) return;

    int8_t x_move = (int8_t)inb(0x60);
    int8_t y_move = (int8_t)inb(0x60);

    mouse_x += x_move;
    mouse_y -= y_move;

    if (mouse_x < 0) mouse_x = 0;
    if (mouse_y < 0) mouse_y = 0;

    update_cursor_position(mouse_x, mouse_y);
}
