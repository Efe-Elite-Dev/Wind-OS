#include "globals.h"
#include "io.h"
#include "gui.h"

static int mouse_x = 100;
static int mouse_y = 100;

void update_cursor_position(int delta_x, int delta_y) {
    mouse_x += delta_x;
    mouse_y -= delta_y; 

    if (mouse_x < 0) mouse_x = 0;
    if (mouse_x >= SCREEN_WIDTH) mouse_x = SCREEN_WIDTH - 1;
    if (mouse_y < 0) mouse_y = 0;
    if (mouse_y >= SCREEN_HEIGHT) mouse_y = SCREEN_HEIGHT - 1;
}

void mouse_handler(void) {
    static uint8_t mouse_cycle = 0;
    // int8_t yerine doğrudan donanımın anlayacağı signed char tipini kullanıyoruz
    static signed char mouse_byte[3];

    uint8_t status = inb(0x64);
    if ((status & 0x01) && (status & 0x20)) {
        mouse_byte[mouse_cycle++] = (signed char)inb(0x60);

        if (mouse_cycle == 3) {
            mouse_cycle = 0;
            
            int delta_x = (int)mouse_byte[1];
            int delta_y = (int)mouse_byte[2];
            
            update_cursor_position(delta_x, delta_y);
            
            // İmleci ekrana kare olarak çiz
            draw_rect(mouse_x, mouse_y, 6, 6, 0xFFFFFF);
        }
    }
}
