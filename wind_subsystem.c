#ifndef WIND_SUBSYSTEM_H
#define WIND_SUBSYSTEM_H

#include <stdint.h>

/* =============================================================================
   DÜŞÜK SEVİYE DONANIM I/O PORT FONKSİYONLARI (ZIRHLI)
   ============================================================================= */

/**
 * @brief Belirtilen I/O portundan 8-bitlik (Byte) veri okur.
 */
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/**
 * @brief Belirtilen I/O portuna 8-bitlik (Byte) veri yazar.
 */
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

/* =============================================================================
   WIND ALT SİSTEMİ MERKEZİ BİLDİRİMLERİ
   ============================================================================= */

void init_wind_subsystem(void);
void wind_subsystem_tick(void);
uint32_t get_wind_subsystem_uptime(void);

/* Grafik ve Pencere Prototipi (gui.c ve exe_subsystem.c için) */
void draw_window_pure(int x, int y, int width, int height, uint32_t border_color);

#endif /* WIND_SUBSYSTEM_H */
