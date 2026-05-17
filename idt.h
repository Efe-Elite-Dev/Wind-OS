#ifndef IDT_H
#define IDT_H

#include <stdint.h>

/* =============================================================================
   DONANIMSAL KESME TABLOSU (IDT) FONKSİYON PROTOTİPLERİ
   =============================================================================
 */

/**
 * @brief Donanımsal Kesme Tablosunu (Interrupt Descriptor Table) ilklendirir.
 * Çekirdeğin (Kernel) işlemci seviyesindeki hataları ve donanım sinyallerini
 * (Klavye, Fare, Zamanlayıcı) yakalamasını sağlayan ana kapıdır.
 */
void init_idt(void);

#endif /* IDT_H */
