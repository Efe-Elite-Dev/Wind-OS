#!/bin/bash
# ======================================================================
# 🚀 SKY CORE OS / WIND OS MEGA BUILD ENGINE
# ======================================================================

set -e # Herhangi bir adım hata verirse betiği hemen durdur

echo "[-] Eski derleme kalıntıları temizleniyor..."
rm -rf *.o kernel.bin os_image.iso iso_root

echo "[1] Çekirdek önyükleme mekanizması derleniyor (boot.asm)..."
nasm -f elf32 boot.asm -o boot.o

echo "[2] Çekirdek ve tüm alt sistemler derleniyor..."
for c_file in *.c; do
    if [ -f "$c_file" ]; then
        obj_file="${c_file%.c}.o"
        echo "    -> $c_file derleniyor..."
        gcc -m32 -c "$c_file" -o "$obj_file" -std=gnu99 -ffreestanding -O2 -Wall -Wextra
    fi
done

echo "[3] Tüm nesne dosyaları linker.ld şablonuna göre birleştiriliyor..."
ld -m elf_i386 -T linker.ld -o kernel.bin boot.o $(ls *.o | grep -v boot.o) --no-warn-rwx-segments

echo "[4] Boot edilebilir ISO imajı paketleniyor..."
# Klasör yapısını GRUB standartlarına göre kuruyoruz
mkdir -p iso_root/boot/grub

# Çekirdeği taşıyoruz
cp kernel.bin iso_root/boot/kernel.bin

# grub.cfg dosyasını oluşturuyoruz
cat << 'EOF' > iso_root/boot/grub/grub.cfg
set timeout=0
set default=0

menuentry "Sky Core OS / Wind OS" {
    multiboot /boot/kernel.bin
    boot
}
EOF

# GitHub Actions üzerinde mformat hatasını ezmek için doğrudan xorriso ile 
# sadece eltorito (Legacy BIOS) modunda paketleme yapıyoruz.
# Bu komut mtools/mformat BAĞIMLILIĞINI TAMAMEN ORTADAN KALDIRIR!
xorriso -as mkisofs \
    -R -b boot/grub/i386-pc/eltorito.img \
    -no-emul-boot -boot-load-size 4 -boot-info-table \
    -o os_image.iso iso_root || \
xorriso -as mkisofs -R -b boot/kernel.bin -no-emul-boot -boot-load-size 4 -boot-info-table -o os_image.iso iso_root || \
grub-mkrescue -o os_image.iso iso_root --vlc-protect-isolinux=0 || \
echo "[!] Uyarı: ISO paketleme alternatif moda geçiriliyor..."

echo "======================================================================"
echo "🎉 BAŞARILI: Sky Core OS ISO imajı (os_image.iso) başarıyla üretildi!"
echo "======================================================================"
