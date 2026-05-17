#include "setup.h"

// Başlık dosyası senkronizasyon hatalarını önlemek için makroları buraya sabitliyoruz
#ifndef COLOR_WIND_3D_LIGHT
#define COLOR_WIND_3D_LIGHT  22  // Parlayan 3D Kenar Işığı (Beyaz)
#endif

#ifndef COLOR_WIND_3D_DARK
#define COLOR_WIND_3D_DARK   23  // Derin 3D Kenar Gölgesi (Koyu Gri)
#endif

extern void outb(uint16_t port, uint8_t val);

static SetupStage current_stage = STAGE_COUNTRY;
static int button_press_depth = 0; // 3D Tuşun içeri gömülme derinliği (0-4 piksel)
static int tunnel_zoom = 0;        // Ekranın içeri doğru akma animasyonu katmanı

void setup_init_palette(void) {
    outb(0x3C8, COLOR_WIND_BG);
    outb(0x3C9, 55); outb(0x3C9, 58); outb(0x3C9, 60); // Klasik 3D Gri Arka Plan

    outb(0x3C8, COLOR_WIND_CARD);
    outb(0x3C9, 63); outb(0x3C9, 63); outb(0x3C9, 63); // Saf Beyaz Panel İç yüzü

    outb(0x3C8, COLOR_WIND_TEXT);
    outb(0x3C9, 0);  outb(0x3C9, 0);  outb(0x3C9, 10);  // Koyu Gece Laciverdi

    outb(0x3C8, COLOR_WIND_PRIMARY);
    outb(0x3C9, 45); outb(0x3C9, 45); outb(0x3C9, 45); // Tuş yüzeyi için nötr gri

    outb(0x3C8, COLOR_WIND_3D_LIGHT);
    outb(0x3C9, 63); outb(0x3C9, 63); outb(0x3C9, 63); // Parlayan 3D Kenar Işığı

    outb(0x3C8, COLOR_WIND_3D_DARK);
    outb(0x3C9, 28); outb(0x3C9, 28); outb(0x3C9, 28); // Derin 3D Kenar Gölgesi
}

void setup_put_pixel(int x, int y, uint8_t color) {
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        VIDEO_MEMORY[y * SCREEN_WIDTH + x] = color;
    }
}

void setup_draw_rect(int x, int y, int w, int h, uint8_t color) {
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            setup_put_pixel(x + j, y + i, color);
        }
    }
}

// Gelişmiş Full 3D Çerçeve Çizici (Windows Klasik İllüzyonu)
void setup_draw_3d_frame(int x, int y, int w, int h, int sunken) {
    if (!sunken) {
        // Dışarı Fırlamış 3D Yapı (Pencereler ve Normal Tuşlar)
        setup_draw_rect(x, y, w, h, COLOR_WIND_PRIMARY);
        setup_draw_rect(x, y, w, 2, COLOR_WIND_3D_LIGHT); // Üst parlaklık
        setup_draw_rect(x, y, 2, h, COLOR_WIND_3D_LIGHT); // Sol parlaklık
        setup_draw_rect(x, y + h - 2, w, 2, COLOR_WIND_3D_DARK);  // Alt gölge
        setup_draw_rect(x + w - 2, y, 2, h, COLOR_WIND_3D_DARK);  // Sağ gölge
    } else {
        // İçeri Oyulmuş 3D Yapı (Giriş Alanları veya Basılı Tuşlar)
        setup_draw_rect(x, y, w, h, COLOR_WIND_BG);
        setup_draw_rect(x, y, w, 2, COLOR_WIND_3D_DARK);  // Üst artık karanlık
        setup_draw_rect(x, y, 2, h, COLOR_WIND_3D_DARK);  // Sol artık karanlık
        setup_draw_rect(x, y + h - 2, w, 2, COLOR_WIND_3D_LIGHT); // Alt ışık alır
        setup_draw_rect(x + w - 2, y, 2, h, COLOR_WIND_3D_LIGHT); // Sağ ışık alır
    }
}

// Gerçek Zamanlı Mekanik Basılma Animasyonlu 3D Buton
void draw_mechanical_button(int x, int y, int w, int h, int depth) {
    // Buton gölgesi (Tuş çöktükçe arkada kalan boşluk hissi)
    setup_draw_rect(x, y, w, h, COLOR_WIND_3D_DARK);
    
    // Tuşun kendisi: depth değeri arttıkça tuş sağa ve aşağı (içeri) kayar!
    int tx = x + depth;
    int ty = y + depth;
    int tw = w - depth;
    int th = h - depth;
    
    if (depth == 0) {
        setup_draw_3d_frame(tx, ty, tw, th, 0);
    } else {
        // Tuş basıldığında kenar çizgileri incelir ve çökmüş moduna geçer
        setup_draw_rect(tx, ty, tw, th, COLOR_WIND_PRIMARY);
        setup_draw_rect(tx, ty, tw, 1, COLOR_WIND_3D_DARK);
        setup_draw_rect(tx, ty, 1, th, COLOR_WIND_3D_DARK);
    }
}

void setup_render(void) {
    // Tüm masaüstünü 3D doku efektli gri yap
    setup_draw_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, COLOR_WIND_BG);
    
    // Tünel/Zoom efekti: tunnel_zoom arttıkça ana panel merkeze doğru büzülür
    int border = 10 + tunnel_zoom;
    setup_draw_3d_frame(border, border, SCREEN_WIDTH - (border * 2), SCREEN_HEIGHT - (border * 2) - 10, 0);
    
    // İç kısımdaki beyaz çalışma alanı da 3D içeri gömülü tasarlanıyor
    setup_draw_3d_frame(border + 10, border + 10, SCREEN_WIDTH - (border * 2) - 20, SCREEN_HEIGHT - (border * 2) - 45, 1);
    setup_draw_rect(border + 12, border + 12, SCREEN_WIDTH - (border * 2) - 24, SCREEN_HEIGHT - (border * 2) - 49, COLOR_WIND_CARD);

    // Mekanik "Devam Et" Butonu
    draw_mechanical_button(220 - tunnel_zoom, 140, 65, 18, button_press_depth);

    // Seçim alanları ve 3D kutular (Sadece animasyon sabitken göster)
    if (tunnel_zoom == 0) {
        switch (current_stage) {
            case STAGE_COUNTRY:
                setup_draw_3d_frame(120, 50, 140, 16, 1); // İçeri gömülü 3D liste elemanı
                break;
            case STAGE_KEYBOARD:
                setup_draw_3d_frame(40, 70, 50, 30, 0);  // Dışarı fırlamış 3D klavye minyatürü
                break;
            case STAGE_NETWORK:
                setup_draw_3d_frame(120, 70, 140, 16, 1); // Gömülü input şifre barı
                break;
            default:
                break;
        }
    }
}

void setup_delay(int count) {
    volatile int i = 0;
    for (i = 0; i < count * 12000; i++) {
        __asm__("nop");
    }
}

// Mekanik Çökme ve İçeri Doğru Akma Geçiş Efekti (Transition)
void trigger_mechanical_transition(void) {
    // 1. Tuş kademeli olarak içeri gömülüyor (Mekanik tuş vuruşu hissi)
    for (button_press_depth = 0; button_press_depth <= 4; button_press_depth++) {
        setup_render();
        setup_delay(10);
    }
    
    // 2. Ekran derinlemesine içeri doğru kayıyor (3D Akış)
    for (tunnel_zoom = 0; tunnel_zoom <= 12; tunnel_zoom += 3) {
        setup_render();
        setup_delay(12);
    }
    
    // Aşama Değişimi
    if (current_stage < STAGE_COMPLETE) {
        current_stage++;
    }
    
    // 3. Yeni ekran tünelden çıkarak eski boyutuna geri yayılıyor
    button_press_depth = 0;
    for (tunnel_zoom = 12; tunnel_zoom >= 0; tunnel_zoom -= 3) {
        setup_render();
        setup_delay(12);
    }
}

void setup_init(void) {
    outb(0x3C2, 0x63);
    uint8_t vga_regs[] = {
        0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
        0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x9C, 0x0E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3, 0xFF
    };
    for (uint8_t i = 0; i < 24; i++) {
        outb(0x3D4, i);
        outb(0x3D5, vga_regs[i]);
    }

    setup_init_palette();
    setup_render();
}

void setup_handle_input(uint8_t scancode) {
    if (scancode == 0x1C) { // ENTER
        trigger_mechanical_transition();
    }
}
