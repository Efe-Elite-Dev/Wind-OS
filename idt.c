#include "idt.h"
#include "wind_subsystem.h"

/* IDT Giriş Yapısı (Interrupt Descriptor Çerçevesi) */
struct idt_entry {
  uint16_t base_low; /* Fonksiyon adresinin düşük 16 biti */
  uint16_t selector; /* Çekirdek kod segment seçicisi (GDT) */
  uint8_t always0;   /* Her zaman 0 kalmalı */
  uint8_t flags; /* Erişim hakları ve kapı tipi (0x8E: 32-bit Interrupt Gate) */
  uint16_t base_high; /* Fonksiyon adresinin yüksek 16 biti */
} __attribute__((packed));

/* İşlemciye fırlatılacak olan IDT Pointer Yapısı */
struct idt_ptr {
  uint16_t limit;
  uint32_t base;
} __attribute__((packed));

/* Toplam 256 adet donanımsal ve yazılımsal kesme kapısı */
static struct idt_entry idt[256];
static struct idt_ptr idtr;

/* Assembly dilinde yazılmış olan IDT yükleyici dış fonksiyonu */
extern void idt_load(uint32_t idt_ptr_addr);

/* Belirli bir kesme kapısını dolduran iç fonksiyon */
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
  idt[num].base_low = (base & 0xFFFF);
  idt[num].base_high = ((base >> 16) & 0xFFFF);
  idt[num].selector = sel;
  idt[num].always0 = 0;
  idt[num].flags = flags;
}

/* Boş kesme koruyucusu (Unhandled interrupts için sistemi kilitlemez) */
void default_interrupt_handler(void) {
  // Gelecekte donanım çökmelerini ayıklamak için içi doldurulabilir
}

/* Merkezi IDT Başlatma Motoru */
void init_idt(void) {
  // 1. IDT işaretçisini (pointer) hazırla
  idtr.limit = (sizeof(struct idt_entry) * 256) - 1;
  idtr.base = (uint32_t)&idt;

  // 2. Tüm kapıları güvenli bir varsayılan fonksiyonla sıfırla
  for (int i = 0; i < 256; i++) {
    idt_set_gate(i, (uint32_t)default_interrupt_handler, 0x08, 0x8E);
  }

  /* NOT: boot.asm içinde tanımladığın "idt_load" fonksiyonunu tetiklemek için
     aşağıdaki Inline Assembly komutunu kullanıyoruz. Eğer boot.asm içinde
     global idt_load varsa harici fonksiyonu çağırır, yoksa bu inline makine
     kodu doğrudan IDTR yazmacını günceller ve sistemi zırhlar!
  */
  __asm__ volatile("lidt %0" : : "m"(idtr));
}
