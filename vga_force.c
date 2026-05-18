// ==============================================================================
//  WIND OS - DONANIMSAL GRAFİK MODU ZORLAYICI (vga_force.c)
// ==============================================================================
#include <stdint.h>

// Bochs/VirtualBox Grafik Port Adresleri
#define VBE_DISPI_IOPORT_INDEX          0x01CE
#define VBE_DISPI_IOPORT_DATA           0x01CF

// VBE Komut Kayıtçısı İndeksleri
#define VBE_DISPI_INDEX_ID              0
#define VBE_DISPI_INDEX_XRES            1
#define VBE_DISPI_INDEX_YRES            2
#define VBE_DISPI_INDEX_BPP             3
#define VBE_DISPI_INDEX_ENABLE          4

// VBE Grafik Kontrol Komutları
#define VBE_DISPI_DISABLED              0x00
#define VBE_DISPI_ENABLED               0x01
#define VBE_DISPI_LFB_ENABLED           0x40 // Linear Framebuffer Modunu Zorla!

// Dışarıdan erişilecek gerçek video adresi
extern uint32_t* GRAPHICS_FRAMEBUFFER;

// İşlemci Port Yazma Fonksiyonu (I/O Port Assembly Köprüsü)
void outw(uint16_t port, uint16_t val) {
    __asm__ volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}

// BGA / VirtualBox Grafik Kayıtçısına Veri Yazma
void vbe_write(uint16_t index, uint16_t value) {
    outw(VBE_DISPI_IOPORT_INDEX, index);
    outw(VBE_DISPI_IOPORT_DATA, value);
}

// 🔥 EKRANI ZORLA AÇAN KRİTİK FONKSİYON
void force_graphics_hardware(void) {
    // 1. Önce grafik modunu sıfırla (Karta reset atıyoruz)
    vbe_write(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_DISABLED);
    
    // 2. Çözünürlüğü ve Bit Derinliğini Donanımsal Portlardan Çakıyoruz
    vbe_write(VBE_DISPI_INDEX_XRES, 1024); // Genişlik: 1024
    vbe_write(VBE_DISPI_INDEX_YRES, 768);  // Yükseklik: 768
    vbe_write(VBE_DISPI_INDEX_BPP, 32);    // 32-bit True Color
    
    // 3. Ekran Kartını "Zorla Grafik Moduna Geç ve Hafızayı Aç" Emriyle Ateşle
    vbe_write(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_ENABLED | VBE_DISPI_LFB_ENABLED);
    
    // 4. VirtualBox standart PCI grafik adresini (0xE0000000) garantiye alıyoruz
    if ((uint32_t)GRAPHICS_FRAMEBUFFER == 0xFD000000 || GRAPHICS_FRAMEBUFFER == 0) {
        GRAPHICS_FRAMEBUFFER = (uint32_t*)0xE0000000; // Standart VirtualBox LFB Adresi
    }
}
