#include "wind_subsystem.h"

/* kernel.c içindeki akıcı arabelleğe dışarıdan bağlanıyoruz */
extern uint32_t* back_buffer;

/* DİKKAT: outb ve inb fonksiyonları wind_subsystem.c içinde zaten 
   tanımlandığı için, linker'ın "multiple definition" (mükerrer tanım) 
   hatası vermesini engellemek amacıyla buradan tamamen KALDIRILDI! 
   Sistem artık merkezi I/O portlarını kullanacak.
*/

void init_graph_mode(void) {
    /* VBE modu GRUB ve boot.asm tarafından otomatik kuruluyor */
}

void draw_pixel_pure(int x, int y, uint32_t color) {
    /* Ekrandan taşma kontrolü (Sanal makine çökmesini engeller) */
    if (x < 0 || x >= 800 || y < 0 || y >= 600) return;
    
    /* Doğrudan 32MB güvenli arabelleğe yaz */
    if (back_buffer != 0) {
        back_buffer[y * 800 + x] = color;
    }
}

void clear_screen_gfx(uint32_t color) {
    for (int y = 0; y < 600; y++) {
        for (int x = 0; x < 800; x++) {
            draw_pixel_pure(x, y, color);
        }
    }
}

void clear_text_screen(void) {
    /* Eski text modundan kalma temizlik fonksiyonu */
    char* vga_text = (char*)0xB8000;
    for (int i = 0; i < 80 * 25 * 2; i += 2) {
        vga_text[i] = ' ';
        vga_text[i+1] = 0x07;
    }
}
