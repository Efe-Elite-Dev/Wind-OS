#include <stdbool.h>

// Dışarıdan (screen.c, gui.c veya keyboard.c'den) gelecek olan fonksiyonların prototipleri
extern void screen_clear(void);
extern void draw_string(const char* text, int x, int y, unsigned int color);
extern void draw_button(const char* label, int x, int y, int width, int height);
extern void draw_rectangle(int x, int y, int width, int height, unsigned int color);
extern void trigger_vortex_kernel_main(void); // Ana işletim sistemi masaüstü tetikleyicisi

// Kurulum ve Çekmece Durum Değişkenleri
int current_setup_step = 1;
bool is_app_drawer_open = true; // Şemadaki gibi varsayılan olarak açık geliyor

void render_setup_wizard(void) {
    // Adım 1: İlk Kurulum Ekranı - Hoş Geldiniz
    if (current_setup_step == 1) {
        screen_clear();
        draw_string("=== ILK KURULUM EKRANI - HOS GELDINIZ ===", 200, 50, 0xFFFFFF);
        
        draw_rectangle(212, 150, 600, 400, 0x241A3E); // Merkezdeki kart paneli
        draw_string("[ SKY CORE OS PUSULA LOGOSU ]", 350, 220, 0x00D2D3);
        draw_string("Sisteme Hos Geldiniz!", 380, 300, 0xFFFFFF);
        
        // Şemadaki alt butonlar
        draw_button("Hizli Kurulum (Az)", 260, 450, 220, 40);
        draw_button("Detaylu Kurulum (Baska)", 520, 450, 240, 40);
    } 
    
    // Adım 2: Konum & Saat Ayarlama
    else if (current_setup_step == 2) {
        screen_clear();
        draw_string("=== KONUM & SAAT AYARLAMA ===", 320, 40, 0xFFFFFF);
        
        // Sol Panel (Teşekkür ve WiFi)
        draw_rectangle(50, 120, 440, 520, 0x241A3E);
        draw_string("SKY CORE OS v1.5'i", 120, 160, 0x1DD1A1);
        draw_string("Sectiginiz Icin Tesekkurler!", 80, 200, 0xFFFFFF);
        draw_string("Wi-Fi A BAGLANA", 160, 300, 0xFFFFFF);
        draw_string("-> Ag Adi (Bagli)", 100, 360, 0x1DD1A1);
        draw_button("Atla", 180, 560, 150, 40);
        
        // Sağ Panel (Harita ve Konum)
        draw_rectangle(534, 120, 440, 520, 0x241A3E);
        draw_rectangle(554, 150, 400, 220, 0x362958); // Temsili harita alanı
        draw_string("[ TURKIYE HARITASI ]", 650, 250, 0xFFFFFF);
        draw_string("Konum: [Istanbul, Turkiye]", 580, 410, 0xFFFFFF);
        draw_string("Bolge Saati: [GMT+03:00]", 580, 450, 0xFFFFFF);
        draw_button("Ayarlari Kaydet ve Ilerle", 584, 560, 340, 45);
    }
    
    // Adım 3: Giriş & Tamamlama
    else if (current_setup_step == 3) {
        screen_clear();
        draw_string("=== GIRIS & TAMAMLAMA ===", 340, 50, 0xFFFFFF);
        
        draw_rectangle(262, 130, 500, 500, 0x241A3E);
        draw_string("Giris Bilgilerini Kontrol Edin.", 310, 180, 0xFFFFFF);
        draw_string("[ AY/GECE LOGOSU - 33.png ]", 320, 280, 0xA29BFE);
        draw_string("Teşekkürler, Kullanima Hazir!", 310, 420, 0xFFFFFF);
        draw_string("Masaustune Gitmek Icin HAZIR", 320, 460, 0x1DD1A1);
        
        draw_button("BASLAT", 387, 560, 250, 45);
    }
    
    // Adım 4: Masaüstü Şeması & Ortadan Çıkan Çekmece
    else if (current_setup_step == 4) {
        screen_clear();
        
        // Masaüstü arka plan rengi
        draw_rectangle(0, 0, 1024, 708, 0x1E1B29);
        
        // Üst orta hava durumu widget'ı
        draw_rectangle(302, 40, 420, 100, 0x2D2640);
        draw_string("26:03 | Istanbul, 18C Bulutlu", 360, 80, 0xFFFFFF);
        
        // Statik Masaüstü Uygulama İkonları (Sol ve Sağ Yanlar)
        draw_string("Kamers\nGaleri", 40, 300, 0xA29BFE);
        draw_string("Mesajlar\nTerminal\nDosyalar", 850, 300, 0xA29BFE);
        
        // TAM ORTADAN ÇIKAN UYGULAMA ÇEKMECESİ
        if (is_app_drawer_open) {
            draw_rectangle(212, 180, 600, 390, 0x2D2942); // Çekmece kutusu
            draw_rectangle(232, 200, 560, 40, 0x1E1B29);   // Arama çubuğu
            draw_string("Ara...", 250, 210, 0x888888);
            
            // Kare Kare Uygulamalar (Şemadaki altı adet uygulama bloğu)
            draw_rectangle(242, 270, 150, 80, 0x3D3659); draw_string("Dosya Yon.", 260, 300, 0xFFFFFF);
            draw_rectangle(437, 270, 150, 80, 0x3D3659); draw_string("Takvim", 470, 300, 0xFFFFFF);
            draw_rectangle(632, 270, 150, 80, 0x3D3659); draw_string("E-Posta", 660, 300, 0xFFFFFF);
            
            draw_rectangle(242, 390, 150, 80, 0x3D3659); draw_string("Tarayici", 260, 420, 0xFFFFFF);
            draw_rectangle(437, 390, 150, 80, 0x3D3659); draw_string("Kod Edit.", 455, 420, 0xFFFFFF);
            draw_rectangle(632, 390, 150, 80, 0x3D3659); draw_string("Ayarlar", 665, 420, 0xFFFFFF);
        }
        
        // GÖREV ÇUBUĞU (TASKBAR)
        draw_rectangle(0, 708, 1024, 60, 0x14111F);
        draw_string("🛠️ ⚙️", 30, 725, 0xFFFFFF); // Sol sistem ikonları
        
        // Ortadaki Tetikleyici Buton alanı
        draw_button("🌀 SKY CORE LOGO", 422, 715, 180, 45);
        
        // Sistemi tamamen başlatıp döngüden çıkaran buton
        draw_button("Sistemi Baslat", 840, 715, 150, 40);
    }
}

// Ortadaki logoya veya "Sistemi Başlat" butonuna tıklanma kontrolü
void handle_center_logo_click(int mouse_x, int mouse_y) {
    if (current_setup_step == 4) {
        // Eğer ortadaki "🌀 SKY CORE LOGO" butonuna basıldıysa çekmeceyi aç/kapat
        if (mouse_x >= 422 && mouse_x <= 602 && mouse_y >= 715 && mouse_y <= 760) {
            is_app_drawer_open = !is_app_drawer_open;
            render_setup_wizard(); // Ekranı güncelle
        }
        
        // Eğer en sağdaki "Sistemi Başlat" butonuna basıldıysa kurulum döngüsünü kır ve kernel'e uç!
        if (mouse_x >= 840 && mouse_x <= 990 && mouse_y >= 715 && mouse_y <= 755) {
            trigger_vortex_kernel_main(); // Döngüyü kıran asıl fonksiyon!
        }
    }
}

// Kurulum adımlarında butonlara basıldığında koordinat kontrolü ile adımları ilerletir
void handle_setup_buttons(int mouse_x, int mouse_y) {
    if (current_setup_step == 1) {
        // Hızlı veya Detaylı butonlarından herhangi birine basılırsa adım 2'ye geç
        if ((mouse_x >= 260 && mouse_x <= 480 && mouse_y >= 450 && mouse_y <= 490) ||
            (mouse_x >= 520 && mouse_x <= 760 && mouse_y >= 450 && mouse_y <= 490)) {
            current_setup_step = 2;
            render_setup_wizard();
        }
    }
    else if (current_setup_step == 2) {
        // Atla veya Kaydet butonuna basılırsa adım 3'e geç
        if ((mouse_x >= 180 && mouse_x <= 330 && mouse_y >= 560 && mouse_y <= 600) ||
            (mouse_x >= 584 && mouse_x <= 924 && mouse_y >= 560 && mouse_y <= 605)) {
            current_setup_step = 3;
            render_setup_wizard();
        }
    }
    else if (current_setup_step == 3) {
        // BAŞLAT butonuna basılırsa masaüstü arayüzüne (Adım 4) geç
        if (mouse_x >= 387 && mouse_x <= 637 && mouse_y >= 560 && mouse_y <= 605) {
            current_setup_step = 4;
            render_setup_wizard();
        }
    }
    else if (current_setup_step == 4) {
        handle_center_logo_click(mouse_x, mouse_y);
    }
}
