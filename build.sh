#!/bin/bash
# ==============================================================================
# 🌪️ Wind OS / Sky Core OS - MEGA BUILD ENGINE & BOOTSTRAPPER (v1.5)
# ==============================================================================
set -e

echo "======================================================================"
echo "🚀 WIND OS MEGA BUILD ENGINE BAŞLATILDI"
echo "======================================================================"
echo "[+] Geçerli Konum: $(pwd)"
echo "[+] Sistem Kaynakları Optimize Ediliyor (Büyük İmaj Desteği Aktif)..."

# 1. Agresif Temizlik Aşaması
echo "[-] Eski derleme kalıntıları temizleniyor..."
# Klasörleri ve tüm .o uzantılı binary kalıntıları kökten kazıyoruz
rm -rf iso_root iso kernel.bin windos.iso build_output.log
rm -f *.o

# 2. Assembly Çekirdeğinin Derlenmesi
echo "[1] Çekirdek önyükleme mekanizması derleniyor (boot.asm)..."
if [ -f "boot.asm" ]; then
    nasm -f elf32 boot.asm -o boot.o
    echo "    -> [OK] boot.o başarıyla üretildi."
else
    echo "    ❌ KRİTİK HATA: boot.asm ana dizinde bulunamadı!"
    exit 1
fi

# 3. Temel C Modüllerinin Derlenmesi (Sıralama Garantili Standart Dizi)
echo "[2] Temel Kernel ve Kurulum modülleri derleniyor..."
# Sıralamanın bozulmaması için Associative Array yerine standart diziye çektik
CORE_SRC_FILES=("kernel.c" "setup.c" "setup_ui.c")
CORE_OBJS=""

for src in "${CORE_SRC_FILES[@]}"; do
    obj="${src%.c}.o"
    if [ -f "$src" ]; then
        echo "    -> Derleniyor: $src"
        gcc -m32 -c "$src" -o "$obj" -std=gnu99 -ffreestanding -O2 -Wall -Wextra
        CORE_OBJS="$CORE_OBJS $obj"
    else
        echo "    ❌ KRİTİK HATA: Ana modül eksik -> $src"
        exit 1
    fi
done

# 4. Gelişmiş Alt Sistemlerin Dinamik ve Eksiksiz Derlenmesi (Mega Liste)
echo "[3] Repodaki tüm gelişmiş alt sistemler zincire mühürleniyor..."
SUBSYSTEMS=(
    "gui"
    "screen"
    "mouse"
    "keyboard"
    "idt"
    "wind_subsystem"
    "exe_subsystem"
    "ai_subsystem"
    "deb_subsystem"
    "uefi_subsystem"
    "si_subsystem"
)

OPTIONAL_OBJS=""
for sys in "${SUBSYSTEMS[@]}"; do
    if [ -f "${sys}.c" ]; then
        echo "    [+] Alt Sistem Yakalandı: ${sys}.c derleniyor..."
        gcc -m32 -c "${sys}.c" -o "${sys}.o" -std=gnu99 -ffreestanding -O2 -Wall -Wextra
        OPTIONAL_OBJS="$OPTIONAL_OBJS ${sys}.o"
    else
        echo "    [-] Bilgi: ${sys}.c bu derlemede pas geçildi (Dosya yok)."
    fi
done

# 5. Bağlama (Linker) Aşaması - MULTIBOOT DUYARLI SIRALAMA
echo "[4] Tüm nesne dosyaları linker.ld şablonuna göre birleştiriliyor..."
if [ -f "linker.ld" ]; then
    # ÖNEMLİ: boot.o her zaman en başta olmalı ki multiboot header en üste mühürlensin!
    ld -m elf_i386 -z noexecstack -T linker.ld -o kernel.bin boot.o $CORE_OBJS $OPTIONAL_OBJS
    echo "    -> [OK] kernel.bin başarıyla mühürlendi."
else
    echo "    ❌ KRİTİK HATA: linker.ld bulunamadı!"
    exit 1
fi

# 6. ISO Dağıtım Havuzunun Hazırlanması (Klasör ismi iso_root olarak izole edildi)
echo "[5] Canlı ISO yapısı ve GRUB önyükleme katmanı yapılandırılıyor..."
mkdir -p iso_root/boot/grub
cp kernel.bin iso_root/boot/kernel.bin

# Vasiyet ettiğin fırtınalı temayı simüle eden GRUB yapılandırması
cat << EOF > iso_root/boot/grub/grub.cfg
set timeout=3
set default=0

# Ekran kartı modu ayarları (VGA Çözünürlük Standartları)
set gfxmode=1024x768x32
set gfxpayload=keep

menuentry "Wind OS / Sky Core OS v1.5 (Vortex Kernel)" {
    multiboot /boot/kernel.bin
    boot
}
EOF

# 7. Büyük Boyutlu Medya Paketleme Aşaması
echo "[6] Önyüklenebilir medya (windos.iso) oluşturuluyor..."
grub-mkrescue -o windos.iso iso_root -- -volid "WINDOS_15" -as mkisofs -iso-level 3

# Temizlik hamlesi: Geçici oluşturulan klasörü arkamızda iz bırakmamak için uçuruyoruz
rm -rf iso_root

if [ -f "windos.iso" ]; then
    ISO_SIZE=$(du -h windos.iso | cut -f1)
    echo "======================================================================"
    echo "✅ BAŞARILI: Wind OS ISO imajı üretime hazır!"
    echo "[+] Çıktı Dosyası: windos.iso"
    echo "[+] İmaj Boyutu: $ISO_SIZE"
    echo "======================================================================"
else
    echo "❌ HATA: ISO oluşturma motoru başarısız oldu!"
    exit 1
fi
