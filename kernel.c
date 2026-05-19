#include "globals.h"
#include "gui.h"

// Varsayılan korumalı LFB adresi
uint32_t* gfx_framebuffer = (uint32_t*)0xE0000000; 

SystemState current_state = STATE_WELCOME;
bool ai_hud_visible = false;
SetupData os_setup_data;

void kernel_main(uint32_t magic, uint32_t* mbi) {
    bool graphics_success = false;

    if (magic == 0x2BADB002 && mbi != NULL) {
        // Multiboot flags içindeki grafik tablosu bitini (bit 12) kontrol et
        if (mbi[0] & (1 << 12)) {
            uint32_t width = mbi[25];
            uint32_t height = mbi[26];
            uint8_t bpp = mbi[27] & 0xFF;
            uint8_t fb_type = (mbi[27] >> 8) & 0xFF;
            
            // YALNIZCA GRUB gerçek bir 800x600 32-bit grafik modu (fb_type == 1) sağladıysa kabul et
            if (fb_type == 1 && width == 800 && height == 600 && bpp == 32) {
                uint32_t real_fb = mbi[22]; 
                if (real_fb != 0) {
                    gfx_framebuffer = (uint32_t*)real_fb;
                    graphics_success = true;
                }
            }
        }
    }

    // KRİTİK KORUMA: Eğer GRUB grafik modunu açamadıysa metin ekranını ezmemek için CPU'yu kilitle!
    if (!graphics_success) {
        while (1) {
            __asm__ __volatile__("cli; hlt");
        }
    }

    init_vga();
    os_setup_data.wifi_connected = false;

    while (true) {
        switch (current_state) {
            case STATE_WELCOME:
                draw_setup_welcome();
                break;
            case STATE_LOCATION:
                draw_setup_location();
                break;
            case STATE_COMPLETING:
                draw_setup_completing();
                break;
            case STATE_DESKTOP:
                draw_main_desktop();
                break;
        }

        if (ai_hud_visible) {
            draw_ai_subsystem_hud();
        }
    }
}
