#include "wind_subsystem.h"
#include "keyboard.h" /* Kendi başlık dosyasını dahil ederek prototipleri kilitliyoruz */

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

/* KLAVYE YAPAY ZEKASI (Yerel Ajan Değişkenleri) */
static int keyboard_stroke_count = 0;
static int keyboard_ai_cadence_score = 0;

void init_keyboard(void) {
    /* İleride PIC kesme denetleyicisi maskeleri buraya kilitlenecek */
    keyboard_stroke_count = 0;
    keyboard_ai_cadence_score = 0;
}

/* KLAVYE YAPAY ZEKASI: Tuşa basma sıklığına göre zamanlayıcıya tempo katsayısı üretir */
int ai_keyboard_analyze_cadence(void) {
    if (keyboard_stroke_count > 18) {
        keyboard_ai_cadence_score = 3; /* Kullanıcı çok hızlı yazıyor, işlemci gecikmesini sıfırla */
    } else if (keyboard_stroke_count > 6) {
        keyboard_ai_cadence_score = 2; /* Akıcı ve normal yazma hızı */
    } else {
        keyboard_ai_cadence_score = 1; /* Klavye boştadır veya beklemededir */
    }
    return keyboard_ai_cadence_score;
}

/* Klavye Tarama Kodu ASCII Dönüşüm Motoru */
char scancode_to_ascii_core(unsigned char scancode) {
    switch(scancode) {
        case 0x1E: return 'A'; case 0x30: return 'B'; case 0x2E: return 'C';
        case 0x20: return 'D'; case 0x12: return 'E'; case 0x21: return 'F';
        case 0x22: return 'G'; case 0x23: return 'H'; case 0x17: return 'I';
        case 0x24: return 'J'; case 0x25: return 'K'; case 0x26: return 'L';
        case 0x32: return 'M'; case 0x31: return 'N'; case 0x18: return 'O';
        case 0x19: return 'P'; case 0x10: return 'Q'; case 0x13: return 'R';
        case 0x1F: return 'S'; case 0x14: return 'T'; case 0x16: return 'U';
        case 0x2F: return 'V'; case 0x11: return 'W'; case 0x2D: return 'X';
        case 0x15: return 'Y'; case 0x2C: return 'Z';
        case 0x39: return ' ';  /* Boşluk */
        case 0x1C: return '\n'; /* Enter */
        default: return 0;
    }
}

/* Klavyeyi polled yöntemiyle dinleyen ve AI verisi üreten saf fonksiyon */
void check_keyboard_pure(void) {
    static unsigned char last_kb_scancode = 0;
    
    /* Klavye durum portunda okunabilir veri var mı? */
    if (inb(KEYBOARD_STATUS_PORT) & 1) {
        unsigned char scancode = inb(KEYBOARD_DATA_PORT);
        
        /* Tuşun basılma (Make) ve bırakılma (Break) kontrolü */
        if (scancode != last_kb_scancode && scancode < 0x80) {
            char ascii = scancode_to_ascii_core(scancode);
            if (ascii != 0) {
                /* Yerel Yapay Zeka ajanını tetikle */
                keyboard_stroke_count += 4;
                
                /* Emniyet kilidi: Sayacın sonsuza gidip taşmasını engelle */
                if (keyboard_stroke_count > 100) keyboard_stroke_count = 100;
            }
        }
        last_kb_scancode = scancode;
    } else {
        /* Yapay zeka sönümlendirme filtresi: Ritim yavaşladıkça sayacı erit */
        if (keyboard_stroke_count > 0) keyboard_stroke_count--;
    }
}
