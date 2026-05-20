#include "kernel.h"

/* Global ve Statik Değişken Tanımlamaları */
static u32* FB        = (u32*)0;
static u32  SCR_W     = 1024;
static u32  SCR_H     = 768;
static u32  SCR_PITCH = 1024;

static OS_State state = STATE_SETUP_1_NAME;

static u8 start_menu_open   = 0;
static u8 file_manager_open = 0;

static i32 mouse_x = 512;
static i32 mouse_y = 384;
static u8  mouse_btn = 0;
static u8  prev_mouse_btn = 0;

static char username[32] = "Efe";
static i32  user_len = 3;

/* Font Tanımı */
static const u8 font8x8[128][8] = {
    // ... font hex verileriniz ...
};

/* Port G/Ç Makroları veya Inline Fonksiyonlar */
static inline u8  inb(u16 p)       { u8  v; __asm__ volatile("inb %1,%0":"=a"(v):"Nd"(p)); return v; }
static inline void outb(u16 p,u8 v){ __asm__ volatile("outb %0,%1"::"a"(v),"Nd"(p)); }
static inline u32 inl(u16 p)       { u32 v; __asm__ volatile("inl %1,%0":"=a"(v):"Nd"(p)); return v; }
static inline void outl(u16 p,u32 v){ __asm__ volatile("outl %0,%1"::"a"(v),"Nd"(p)); }

/* Fonksiyonların Gerçek Gövdeleri */
void put_pixel(i32 x, i32 y, u32 color){
    // piksel çizme kodları...
}

void fill_rect(i32 x, i32 y, i32 w, i32 h, u32 color){
    // dikdörtgen doldurma kodları...
}

/* ... Diğer tüm fonksiyonların (screen_setup_1, mouse_poll vb.) gövdeleri ... */

/* Ana Giriş Noktası */
void kernel_main(multiboot_info_t* mbi){
    FB        = (u32*)(unsigned long)mbi->framebuffer_addr;
    SCR_W     = mbi->framebuffer_width;
    SCR_H     = mbi->framebuffer_height;
    SCR_PITCH = mbi->framebuffer_pitch / 4;

    if(!FB || SCR_W==0){
        FB       = (u32*)0xFD000000u;
        SCR_W    = 1024;
        SCR_H    = 768;
        SCR_PITCH= 1024;
    }

    mouse_init();
    pci_scan();

    while(1){
        mouse_poll();
        u8 key = kbd_poll();

        switch(state){
          case STATE_SETUP_1_NAME:      screen1(key);  break;
          case STATE_SETUP_2_REGION:    screen2();     break;
          // ... diğer case durumları ...
        }
    }
}
