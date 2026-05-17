#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

/* =============================================================================
   AKILLI KLAVYE SÜRÜCÜSÜ FONKSİYON PROTOTİPLERİ
   =============================================================================
 */

/* Klavye donanımını ve stres/ritim tamponlarını ilklendirir */
void init_keyboard(void);

/* Klavyeden gelen kesmeleri veya ham port verilerini sorgular */
void check_keyboard_pure(void);

/* YAPAY ZEKA ENTEGRASYONU: Yazma ritmini (cadence) analiz ederek AI motoruna
 * besler */
int ai_keyboard_analyze_cadence(void);

#endif /* KEYBOARD_H */
