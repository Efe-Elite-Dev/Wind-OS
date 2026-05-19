#include "sky_core.h"

void kpanic(uint8_t error_code, const char* message) {
    (void)error_code;
    (void)message;
    // Hata durumunda FRAMEBUFFER'ı kullanarak ekrana kırmızı bir uyarı basabilirsin
    if(FRAMEBUFFER) {
        draw_rect(0, 0, 100, 100, 0xFFFF0000); // Ekranın sol üstünde kırmızı kare
    }
    while(1);
}
