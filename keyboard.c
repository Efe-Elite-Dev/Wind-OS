#include "wind_subsystem.h"

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

static uint8_t last_scancode = 0;

/**
 * @brief Klavye sürücüsünü ve donanım tamponunu ilk fırlatan motor.
 */
void init_keyboard(void) {
  last_scancode = 0;
  // Donanım tamponunu temizlemek için boşa bir okuma yapıyoruz
  (void)inb(KEYBOARD_DATA_PORT);
}

/**
 * @brief Klavye tampon belleğinde yeni tuş basımı var mı kontrol eder.
 */
void check_keyboard_pure(void) {
  uint8_t status = inb(KEYBOARD_STATUS_PORT);

  // En düşük bit 1 ise tamponda okunmayı bekleyen veri vardır
  if (status & 0x01) {
    last_scancode = inb(KEYBOARD_DATA_PORT);
  }
}

/**
 * @brief Okunan en son donanım tarama kodunu döner.
 */
uint8_t get_last_scancode(void) { return last_scancode; }
