#include "kernel.h"

/* =========================================================
   GLOBAL VE STATİK DEĞİŞKENLER
   (kernel.h içinde bulunmayan, sadece kernel.c'ye özel olanlar)
   ========================================================= */
static u32* FB        = (u32*)0;
static u32  SCR_W     = 1024;
static u32  SCR_H     = 768;
static u32  SCR_PITCH = 1024;

static OS_State state = STATE_SETUP_1_NAME;

static u8 start_menu_open   = 0;
static u8 file_manager_open = 0;

static i32 mouse_x = 512;
static i32 mouse_y = 384;

/* * NOT: mouse_btn, prev_mouse_btn, username, user_len, font8x8, 
 * inb, outb, put_pixel ve fill_rect zaten kernel.h içinde tanımlı 
 * olduğu için çakışma (redefinition) hatası vermemesi adına buraya tekrar eklenmedi.
 */

/* =========================================================
   PORT G/Ç SATIR-İÇİ (INLINE) FONKSİYONLARI
   ========================================================= */
static inline u32 inl(u16 p)       { u32 v; __asm__ volatile("inl %1,%0":"=a"(v):"Nd"(p)); return v; }
static inline void outl(u16 p,u32 v){ __asm__ volatile("outl %0,%1"::"a"(v),"Nd"(p)); }

/* =========================================================
   SÜRÜCÜ VE POLLING FONKSİYONLARI (Linker Hatalarını Çözen Kısım)
   ========================================================= */

void mouse_init(void) {
    u32 timeout = 100000;
    
    // PS/2 Fareyi aktifleştirme komut dizisi
    while(timeout--) { if((inb(0x64) & 2) == 0) break; }
    outb(0x64, 0xA8); 
    
    timeout = 100000;
    while(timeout--) { if((inb(0x64) & 2) == 0) break; }
    outb(0x64, 0x20);
    
    // Kendi farenizin ek veri paket hızı ayarları varsa buraya ekleyebilirsiniz.
}

void mouse_poll(void) {
    // Klavye/Fare kontrolcüsünde veri var mı?
    if (inb(0x64) & 1) {
        // Gelen veri fareye mi ait? (0x20 biti kontrolü)
        if (inb(0x64) & 0x20) {
            u8 status = inb(0x60);
            i8 rel_x  = (i8)inb(0x60);
            i8 rel_y  = (i8)inb(0x60);

            mouse_x += rel_x;
            mouse_y -= rel_y; // PS/2 koordinat sisteminde Y ekseni terstir

            // Fare imlecinin ekrandan dışarı taşmasını engelle
            if(mouse_x < 0) mouse_x = 0;
            if(mouse_y < 0) mouse_y = 0;
            if(mouse_x > (i32)SCR_W - 1) mouse_x = SCR_W - 1;
            if(mouse_y > (i32)SCR_H - 1) mouse_y = SCR_H - 1;

            // Değişkenler kernel.h'tan otomatik gelir
            prev_mouse_btn = mouse_btn;
            mouse_btn = (status & 0x07);
        }
    }
}

u8 kbd_poll(void) {
    // Klavyede basılı tuş var mı kontrolü
    if (inb(0x64) & 1) {
        // Fareden gelmiyorsa klavye scancode'unu oku
        if (!(inb(0x64) & 0x20)) {
            return inb(0x60);
        }
    }
    return 0; 
}

void pci_scan(void) {
    // PCI veri yollarını tarayan gövde. 
    // Linker'ın hata vermemesi için şimdilik içi boş bırakıldı, mantığını ekleyebilirsin.
}

/* =========================================================
   KURULUM (SETUP) VE ARAYÜZ FONKSİYONLARI
   ========================================================= */

void screen1(u8 key) {
    // STATE_SETUP_1_NAME: Kullanıcı adı giriş ekranı çizimleri ve mantığı
    // kernel.h içindeki fill_rect, put_pixel ve font fonksiyonlarını doğrudan kullanabilirsin.
}

void screen2(void) {
    // STATE_SETUP_2_REGION: Bölge seçimi ekranı
}

/* =========================================================
   ANA GİRİŞ NOKTASI (KERNEL MAIN)
   ========================================================= */
void kernel_main(multiboot_info_t* mbi){
    // GRUB Multiboot yapısından 32-bit uyumlu pointer eşitlemesi yapıyoruz
    FB        = (u32*)(u32)mbi->framebuffer_addr;
    SCR_W     = mbi->framebuffer_width;
    SCR_H     = mbi->framebuffer_height;
    SCR_PITCH = mbi->framebuffer_pitch / 4; // Satır byte genişliğini u32 piksele çeviriyoruz

    // GRUB veri aktaramadıysa veya adres geçersizse VESA Fallback moduna geç
    if(!FB || SCR_W == 0){
        FB        = (u32*)0xFD000000u;
        SCR_W     = 1024;
        SCR_H     = 768;
        SCR_PITCH = 1024;
    }

    // İlklendirme fonksiyonları çağrılıyor
    mouse_init();
    pci_scan();

    // Sonsuz İşletim Sistemi Döngüsü
    while(1){
        mouse_poll();
        u8 key = kbd_poll();

        switch(state){
            case STATE_SETUP_1_NAME:    
                screen1(key);  
                break;
            case STATE_SETUP_2_REGION:  
                screen2();     
                break;
            case STATE_SETUP_3_KEYBOARD:
                // screen3();
                break;
            case STATE_SETUP_4_NETWORK:
                // screen4();
                break;
            case STATE_SETUP_5_PRIVACY:
                // screen5();
                break;
            case STATE_SETUP_6_UPDATE:
                // screen6();
                break;
            case STATE_SETUP_7_FINISH:
                // screen7();
                break;
            case STATE_DESKTOP:
                // Masaüstü çizim kodları...
                break;
        }
    }
}
