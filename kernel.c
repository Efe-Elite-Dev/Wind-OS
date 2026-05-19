/**
 * ==============================================================================
 * 🌟 SKY CORE OS / WIND OS OPERATING SYSTEM 🌟
 * ==============================================================================
 * [Proje Kodu]: Sky Core OS v1.5 (Vortex TUI Kernel)
 * [Mimari]: x86 Intel/AMD IA-32 Monolitik Çekirdek Standartları
 * [Derleme Hedefi]: 32-Bit Korumalı Mod (Protected Mode)
 * [Arayüz Modu]: Saf Metin Modu Arayüzü (Text User Interface - TUI)
 * ==============================================================================
 */

#include <stdint.h>
#include <stddef.h>

// Sistem Durum Makinesi
typedef enum {
    STATE_WELCOME = 0,    // Ekran 1: İlk Kurulum Ekranı - Hoş Geldiniz
    STATE_LOCATION = 1,   // Ekran 2: Konum & Saat Ayarlama (Türkiye Haritası)
    STATE_SUMMARY = 2,    // Ekran 3: Giriş & Tamamlama (33.png ve Başlat)
    STATE_DESKTOP = 3     // Ekran 4: Widget'lı Fırtına Temalı Masaüstü!
} OS_UI_STATE;

volatile OS_UI_STATE current_os_state = STATE_WELCOME;

// Metin modu video bellek adresi (80x25 karakter)
uint16_t* const TEXT_VIDEO_MEMORY = (uint16_t*)0xB8000;
int text_x = 0;
int text_y = 0;

// Diğer kerror ve vga_force dosyalarının linker'da patlamaması için sembolik tanım
uint32_t* GRAPHICS_FRAMEBUFFER = NULL;

// Port I/O
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Gelişmiş Ekrana Yazdırma Motoru (Renk Destekli)
// renk: Üst 4 bit arka plan, alt 4 bit ön plan (Örn: 0x1F -> Mavi Arka Plan, Beyaz Yazı)
void print_string_color(const char* str, uint8_t color) {
    while (*str) {
        if (*str == '\n') {
            text_x = 0;
            text_y++;
            if (text_y >= 25) text_y = 0;
        } else {
            TEXT_VIDEO_MEMORY[text_y * 80 + text_x] = (color << 8) | *str;
            text_x++;
            if (text_x >= 80) { 
                text_x = 0; 
                text_y++; 
                if (text_y >= 25) text_y = 0;
            }
        }
        str++;
    }
}

// Koordinat odaklı yazı yazdırma (Arayüz pencereleri için)
void draw_text_at(int x, int y, const char* str, uint8_t color) {
    int old_x = text_x;
    int old_y = text_y;
    text_x = x;
    text_y = y;
    print_string_color(str, color);
    text_x = old_x;
    text_y = old_y;
}

// Ekranı komple belirli bir renkle temizleme
void clear_screen_with_color(uint8_t color) {
    for (int i = 0; i < 80 * 25; i++) {
        TEXT_VIDEO_MEMORY[i] = (color << 8) | ' ';
    }
    text_x = 0;
    text_y = 0;
}

// ==============================================================================
// 🖼️ TERMİNAL ARAYÜZÜ RENDER MOTORLARI (GÖRSEL ÇIKTILARA BİREBİR UYARLANDI)
// ==============================================================================

// EKRAN 1: İLK KURULUM EKRANI - HOŞ GELDİNİZ
void render_welcome_screen(void) {
    clear_screen_with_color(0x1F); // Lacivert/Mavi Arka Plan
    
    // Üst Bar
    for(int i=0; i<80; i++) draw_text_at(i, 0, " ", 0x70);
    draw_text_at(2, 0, "Sky Core OS v1.5 - Ilk Kurulum Sihirbazi", 0x70);

    // Ana Pencere
    for(int y=4; y<20; y++) {
        for(int x=10; x<70; x++) {
            draw_text_at(x, y, " ", 0xF0); // Beyaz Panel
        }
    }
    
    draw_text_at(36, 6, "  /\\  ", 0xF1);
    draw_text_at(36, 7, " /  \\ ", 0xF1);
    draw_text_at(36, 8, "/____\\", 0xF1);
    draw_text_at(35, 9, "[PUSULA]", 0xF1);

    draw_text_at(28, 11, "ILK KURULUM EKRANI", 0xF0);
    draw_text_at(27, 12, "Sisteme Hos Geldiniz!", 0xF0);

    // Butonlar
    draw_text_at(15, 16, " [ Hizli Kurulum (Az) ] ", 0x1F);
    draw_text_at(43, 16, " [ Detayli Kurulum (Baska) ] ", 0x70);

    // Alt Bar Bilgilendirmesi
    draw_text_at(18, 23, "Devam etmek icin [ENTER] tusuna basin...", 0x1F);
}

// EKRAN 2: KONUM & SAAT AYARLAMA (TÜRKİYE HARİTASI)
void render_location_screen(void) {
    clear_screen_with_color(0x1F); 

    for(int i=0; i<80; i++) draw_text_at(i, 0, " ", 0x70);
    draw_text_at(2, 0, "KONUM & SAAT AYARLAMA", 0x70);

    // Sol Panel: Bilgiler
    for(int y=3; y<21; y++) {
        for(int x=2; x<32; x++) draw_text_at(x, y, " ", 0x8F);
    }
    draw_text_at(4, 5, "SKY CORE OS v1.5", 0x8F);
    draw_text_at(4, 6, "Sectiginiz Icin", 0x8F);
    draw_text_at(4, 7, "Tesekkurler!", 0x8F);
    draw_text_at(4, 10, "Ag Baglantisi:", 0x8F);
    draw_text_at(4, 11, "Lutfen Bir Aga Baglanin.", 0x87);
    draw_text_at(4, 14, "[ WiFi A BAGLAN ]", 0x1F);
    draw_text_at(4, 17, "[ Atla ]", 0x70);

    // Sağ Panel: Türkiye ASCII Haritası
    draw_text_at(36, 4, "TURKIYE VECTOR MAP", 0x1E);
    draw_text_at(36, 6, "   __...--''''--...__   ", 0x1E);
    draw_text_at(36, 7, " .'.  * ISTANBUL     '. ", 0x1E);
    draw_text_at(36, 8, " :                    : ", 0x1E);
    draw_text_at(36, 9, "  '.                .'  ", 0x1E);
    draw_text_at(36, 10, "    '--...____...--'    ", 0x1E);

    draw_text_at(36, 13, "Konum: [Istanbul, Turkiye]", 0x1F);
    draw_text_at(36, 14, "Bolge Saati: [GMT+03:00]", 0x1F);
    
    draw_text_at(36, 18, " [ Konum ve Saat Ayarlarini Kaydet ] ", 0xF0);
    draw_text_at(18, 23, "Ilerlemek icin [ENTER] tusuna basin...", 0x1F);
}

// EKRAN 3: GİRİŞ & TAMAMLAMA
void render_summary_screen(void) {
    clear_screen_with_color(0x1F);

    // Sol taraftaki ufak harita özeti
    draw_text_at(2, 3, " [ Harita Ozeti ]", 0x1E);
    draw_text_at(2, 5, " Konum: Istanbul", 0x1F);
    draw_text_at(2, 6, " Saat : GMT+03:00", 0x1F);

    // Sağ büyük Giriş & Tamamlama Kutusu
    for(int y=2; y<22; y++) {
        for(int x=25; x<75; x++) draw_text_at(x, y, " ", 0x5F); // Mor/Eflatun Panel
    }

    draw_text_at(38, 4, "GIRIS & TAMAMLAMA", 0x5B); // Cyan renkli başlık
    draw_text_at(32, 6, "Giris Bilgilerini Kontrol Edin.", 0x5F);

    // 33.png Ay Logosu Simülasyonu
    draw_text_at(44, 9, "  _.._  ", 0x5E);
    draw_text_at(44, 10, ".' .-'  [33.png]", 0x5E);
    draw_text_at(44, 11, ":  :    ", 0x5E);
    draw_text_at(44, 12, "'. '._  ", 0x5E);
    draw_text_at(44, 13, "  '--'  ", 0x5E);

    draw_text_at(33, 15, "Tesekkurler, Kullanima Hazir!", 0x5F);
    draw_text_at(34, 16, "Masaustune Gitmek Icin HAZIR", 0x57);

    draw_text_at(42, 19, " [ BASLAT ] ", 0xF0);
    draw_text_at(18, 23, "Sistemi baslatmak icin [ENTER] tusuna basin!", 0x1F);
}

// EKRAN 4: VORTEX MASAÜSTÜ (MÜKEMMEL SONUÇ)
void render_desktop_screen(void) {
    clear_screen_with_color(0x07); // Siyah Arka Plan, Gri Yazılar (Klasik Terminal Altyapısı)

    // Sol Üst Hava Durumu Widget'ı
    for(int y=1; y<7; y++) {
        for(int x=2; x<45; x++) draw_text_at(x, y, " ", 0x1F);
    }
    draw_text_at(4, 2, "SAAT: 26:03", 0x1B);
    draw_text_at(4, 3, "Hava Durumu ve Saat - Esenyurt", 0x1F);
    draw_text_at(4, 4, "21C - Bulutlu ve Firtina", 0x17);

    // Sol Orta Uygulama İkonları
    draw_text_at(2, 9,  "[ Kainera ]", 0x0F);
    draw_text_at(2, 12, "[ Kamera ]", 0x0F);

    // Sağ Panel: Uygulama Çekmecesi
    for(int y=1; y<20; y++) {
        for(int x=55; x<78; x++) draw_text_at(x, y, " ", 0x70);
    }
    draw_text_at(59, 2, "UYGULAMALAR", 0x73);
    draw_text_at(57, 5,  " > Terminal", 0x70);
    draw_text_at(57, 8,  " > Mesajlar", 0x70);
    draw_text_at(57, 11, " > Dosya Yoneticisi", 0x70);
    draw_text_at(57, 14, " > Haritalar", 0x70);

    // Ortadaki Karşılama Popup Kutusu (HOS GELDINIZ)
    for(int y=9; y<15; y++) {
        for(int x=15; x<52; x++) draw_text_at(x, y, " ", 0xF0);
    }
    draw_text_at(24, 10, "HOS GELDINIZ", 0xF0);
    draw_text_at(20, 11, "Sisteme Hos Geldiniz!", 0xF0);
    draw_text_at(24, 13, "GHHOD GERLDIN!", 0xF9); // Mavi parlama efekti

    // Alt Dock Alanı
    for(int x=2; x<52; x++) draw_text_at(x, 21, " ", 0x1F);
    draw_text_at(4, 21, "TUSUMANA BASINCA CEKMECE ACILSIN", 0x1F);

    // Kernel Bilgi Satırı
    draw_text_at(0, 24, "skycore@kernel:~$ Wind OS monolitik cekirdek aktif durumdadir.", 0x0A);
}

// Ekran tazeleme merkezi yönlendiricisi
void refresh_system_display(void) {
    switch (current_os_state) {
        case STATE_WELCOME:  render_welcome_screen();  break;
        case STATE_LOCATION: render_location_screen(); break;
        case STATE_SUMMARY:  render_summary_screen();  break;
        case STATE_DESKTOP:  render_desktop_screen();  break;
    }
}

// İleri ve Geri Gitme Mekanizmaları
void advance_os_stage(void) {
    if (current_os_state < STATE_DESKTOP) {
        current_os_state++;
        refresh_system_display();
    }
}

void regress_os_stage(void) {
    if (current_os_state > STATE_WELCOME && current_os_state != STATE_DESKTOP) {
        current_os_state--;
        refresh_system_display();
    }
}

// ==============================================================================
// 🚀 KERNEL MAIN GİRİŞ NOKTASI
// ==============================================================================
void kernel_main(void* mboot_ptr, uint32_t magic) {
    (void)mboot_ptr;
    (void)magic;

    // Sistem açılır açılmaz direkt ilk kurulum ekranını yükle (Terminal modu kilidi!)
    current_os_state = STATE_WELCOME;
    refresh_system_display();

    // Giriş kontrol döngüsü
    while (1) {
        if (inb(0x64) & 1) { 
            uint8_t scancode = inb(0x60);
            if (!(scancode & 0x80)) { // Tuşa basıldığında
                if (scancode == 0x1C) {       // ENTER TUŞU -> Sonraki Ekran
                    advance_os_stage();
                }
                else if (scancode == 0x0E) {  // BACKSPACE TUŞU -> Önceki Ekran
                    regress_os_stage();
                }
            }
        }
        __asm__ volatile("hlt"); 
    }
}

// ==============================================================================
// 🛠️ LINKER SUSTURUCU KÖPRÜLER (STUBS)
// ==============================================================================
void idt_init(void) {}
void keyboard_init(void) {}
void mouse_init(void) {}
void wind_subsystem_init(void) {}
void exe_subsystem_init(void) {}
void ai_subsystem_init(void) {}
void deb_subsystem_init(void) {}
