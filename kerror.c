#include "globals.h"
#include "gui.h"

void kpanic(void) {
    // Eski GRAPHICS_FRAMEBUFFER makrosu tamamen kaldırıldı.
    // Yeni v1.5 mimarisindeki güvenli grafik motoru fonksiyonlarını tetikliyoruz:
    draw_rect(150, 200, 500, 150, 0xAA0000); // Kırmızı Panik Arka Planı
    
    draw_text("!!! KERNEL PANIC !!!", 280, 230, 0xFFFFFF);
    draw_text("Sistem Kritik Bir Hata Nedeniyle Durduruldu.", 170, 270, 0xFFFFFF);
    
    // İşlemciyi kesmelere kapat ve sonsuz döngüde kilitle
    while (1) {
        __asm__ __volatile__("cli; hlt");
    }
}
