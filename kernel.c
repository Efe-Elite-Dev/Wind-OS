#include "ai_subsystem.h"
#include "wind_subsystem.h"

/* =============================================================================
   MULTIBOOT 1 ÖZELLİK YAPISI (GRUB'DAN GELEN VERİLERİ OKUMAK İÇİN)
   =============================================================================
 */
struct multiboot_info {
  uint32_t flags;
  uint32_t mem_lower;
  uint32_t mem_upper;
  uint32_t boot_device;
  uint32_t cmdline;
  uint32_t mods_count;
  uint32_t mods_addr;
  uint32_t syms[4];
  uint32_t mmap_length;
  uint32_t mmap_addr;
  uint32_t drives_length;
  uint32_t drives_addr;
  uint32_t config_table;
  uint32_t boot_loader_name;
  uint32_t apm_table;
  uint32_t vbe_control_info;
  uint32_t vbe_mode_info;
  uint16_t vbe_mode;
  uint16_t vbe_interface_seg;
  uint16_t vbe_interface_off;
  uint16_t vbe_interface_len;
  // VBE Grafik Modu Alanları (GOP/Linear Framebuffer)
  uint64_t framebuffer_addr;
  uint32_t framebuffer_pitch;
  uint32_t framebuffer_width;
  uint32_t framebuffer_height;
  uint8_t framebuffer_bpp;
  uint8_t framebuffer_type;
  uint8_t color_info[6];
};

/* =============================================================================
   YAPAY ZEKA MERKEZİ SİNİR AĞI HÜCRESİ STRUCT TANIMI
   =============================================================================
 */
struct ai_network_core {
  int predicted_cpu_load;
  int anomaly_flag;
  int system_policy;
  int keyboard_cadence;
};

static struct ai_network_core ai_net;

/* =============================================================================
   HARİCİ ALT SİSTEM VE SÜRÜCÜ BAĞLANTILARI (EXTERNALS)
   =============================================================================
 */
extern void init_idt(void);
extern void init_keyboard(void);
extern void init_mouse(void);
extern void init_graph_mode(void);
extern void clear_screen_gfx(uint32_t color);
extern void handle_mouse_polling(void);
extern void check_keyboard_pure(void);
extern void run_exe_subsystem(void);
extern void gui_refresh_desktop(void);

/* Grafik Ekran Bellek İşaretçileri */
uint32_t *vbe_vram = (uint32_t *)0xFD000000; // Varsayılan yedek adres
uint32_t vbe_pitch = 800 * 4;

/**
 * @brief Klavye basım temposunu ve hızını analiz eden mikro AI fonksiyonu.
 */
int ai_keyboard_analyze_cadence(void) {
  // Klavye temposunu simüle eden temel regresyon hücresi
  return 12;
}

/**
 * @brief Yapay Zeka Çekirdek İnferans Döngüsü
 */
void execute_micro_ai_inference(int loop_count) {
  // 1. Sürücülerden gelen canlı verileri sinir ağı hücrelerine topla
  ai_net.keyboard_cadence = ai_keyboard_analyze_cadence();

  // 2. Mikro Karar Ağacı Simülasyonu
  if (ai_net.keyboard_cadence > 50) {
    ai_net.predicted_cpu_load = 85; // Ağır tempo yük tahmini
    ai_net.anomaly_flag = 0;
    ai_net.system_policy = 1; // Ultra Performans Modu
  } else {
    ai_net.predicted_cpu_load = 15; // Rutin yük tahmini
    ai_net.anomaly_flag = 0;
    ai_net.system_policy = 0; // Dengeli Standart Mod
  }

  // 3. Hesaplanan sinir ağ verilerini ana zamanlayıcıya (scheduler) enjekte et
  ai_core_predict_scheduler(ai_net.predicted_cpu_load, ai_net.anomaly_flag,
                            ai_net.system_policy);

  (void)loop_count;
}

/* =============================================================================
   WIND OS ANA ÇEKİRDEK GİRİŞ NOKTASI (BOOT.ASM BURAYA ATLAR)
   =============================================================================
 */
void kernel_main(struct multiboot_info *mboot) {
  // 1. GRUB'dan gelen VBE Linear Framebuffer bilgilerini doğrula ve bağla
  if (mboot != 0 && (mboot->flags & (1 << 12)) &&
      (mboot->framebuffer_addr != 0)) {
    vbe_vram = (uint32_t *)(uintptr_t)mboot->framebuffer_addr;
    vbe_pitch = mboot->framebuffer_pitch;
  }

  // 2. Donanım Kapılarını ve Temel Alt Sistemleri Ayağa Kaldır
  init_idt();
  init_keyboard();
  init_mouse();
  init_graph_mode();

  // Rüzgar Alt Sistemini Tetikle
  init_wind_subsystem();

  // 3. Ekranı Wind OS Gece Mavisi ile Temizle
  clear_screen_gfx(0x000B1E36);

  int main_loop_counter = 0;

  // 4. SONSUZ ÇEKİRDEK ANA DÖNGÜSÜ (KERNEL MAIN LOOP)
  while (1) {
    main_loop_counter++;

    // Donanım Polling Girişlerini Sorgula
    handle_mouse_polling();
    check_keyboard_pure();

    // Rüzgar Alt Sistemi Saatini Tiklet
    wind_subsystem_tick();

    // Yapay Zeka Çekirdeğini İnferans Modunda Koştur
    if (main_loop_counter % 100 == 0) {
      execute_micro_ai_inference(main_loop_counter);
    }

    // Uygulama ve Grafik Alt Sistemini Canlı Besle
    run_exe_subsystem();
    gui_refresh_desktop();

    // CPU'yu boşa yormamak için mikro bekleme (Donanımsal hlt simülasyonu)
    __asm__ volatile("hlt");
  }
}
