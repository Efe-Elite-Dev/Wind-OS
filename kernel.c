#include "kernel.h"

/* =========================================================
   GLOBAL VE STATİK DEĞİŞKENLER
   (kernel.h içinde TANIMLANMAMIŞ, sadece kernel.c'ye özel olanlar)
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

/* * NOT: mouse_btn, prev_mouse_btn, username ve user_len değişkenleri 
 * zaten kernel.h (Satır 219, 307 vb.) içinde tanımlı olduğu için 
 * derleme hatası almamak adına buradan kaldırıldılar. 
 * Doğrudan o değişkenleri kod içinde kullanmaya devam edebilirsin.
 */

/* =========================================================
   PORT G/Ç SATIR-İÇİ (INLINE) FONKSİYONLARI
   ========================================================= */
/* inb ve outb kernel.h içinde olduğu için sadece inl ve outl burada kalıyor */
static inline u32 inl(u16 p)       { u32 v; __asm__ volatile("inl %1,%0":"=a"(v):"Nd"(p)); return v; }
static inline void outl(u16 p,u32 v){ __asm__ volatile("outl %0,%1"::"a"(v),"Nd"(p)); }

/* * NOT: font8x8, put_pixel ve fill_rect fonksiyonları da 
 * kernel.h (Satır 60, 157, 162) içinde gövdeleriyle birlikte 
 * yer aldığından kernel.c içinde tekrar tanımlanmalarına gerek yoktur.
 */

/* =========================================================
   SİSTEM EKRANLARI VE YARDIMCI FONKSİYONLAR
   ========================================================= */
void screen1(u8 key) {
    // Kurulum Adım 1: Kullanıcı adı giriş ekranı mantığı ve çizimleri
}

void screen2(void) {
    // Kurulum Adım 2: Bölge seçimi ekranı mantığı ve çizimleri
}

/* ... Diğer tüm fonksiyonlarının (screen3, mouse_poll, pci_scan vb.) gövdelerini buraya ekleyebilirsin ... */

/* =========================================================
   ANA GİRİŞ NOKTASI (KERNEL MAIN)
   ========================================================= */
void kernel_main(multiboot_info_t* mbi){
    // GRUB Multiboot üzerinden Framebuffer bilgilerini güvenli tür dönüşümüyle alıyoruz
    FB        = (u32*)(u32)mbi->framebuffer_addr;
    SCR_W     = mbi->framebuffer_width;
    SCR_H     = mbi->framebuffer_height;
    SCR_PITCH = mbi->framebuffer_pitch / 4; // Byte cinsinden genişliği piksele çeviriyoruz

    // Eğer GRUB veri aktaramadıysa yedek (Fallback) VESA adresini ata
    if(!FB || SCR_W == 0){
        FB        = (u32*)0xFD000000u;
        SCR_W     = 1024;
        SCR_H     = 768;
        SCR_PITCH = 1024;
    }

    // Donanımları ilklendir
    mouse_init();
    pci_scan();

    // Sonsuz İşletim Sistemi Döngüsü
    while(1){
        mouse_poll();
        u8 key = kbd_poll();

        // Durum makinesine göre ekranları render et
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
                // Masaüstü çizim fonksiyonu buraya gelecek
                break;
        }
    }
}
