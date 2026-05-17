#ifndef AI_SUBSYSTEM_H
#define AI_SUBSYSTEM_H

/* =============================================================================
   YAPAY ZEKA ALT SİSTEMİ (AI ENGINE) PROTOTİPLERİ
   =============================================================================
 */

/**
 * @brief Fare deltasını ve döngü sayılarını analiz ederek donanım üzerindeki
 * anlık yük tahminini hesaplar.
 */
int ai_predict_hardware_load(int mouse_delta_x, int loop_count);

/**
 * @brief MERKEZİ ENTEGRASYON: kernel.c içindeki karar ağacından gelen verileri
 * alarak çekirdeğin ana zamanlayıcısına (Scheduler) dinamik kuantum sürelerini
 * enjekte eder.
 * * @param predicted_load Tahmin edilen % CPU yükü
 * @param anomaly_flag Donanımsal sapma/hata alarmı (0 veya 1)
 * @param policy Sistem güç/performans politikası (0: Standart, 1: Performans,
 * 2: Güç Koruma)
 */
void ai_core_predict_scheduler(int predicted_load, int anomaly_flag,
                               int policy);

#endif /* AI_SUBSYSTEM_H */
