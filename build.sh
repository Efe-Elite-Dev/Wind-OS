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
# Var olan tüm C dosyalarını otomatik veya manuel derle
gcc -m32 -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
gcc -m32 -c screen.c -o screen.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || echo "[-] screen.c bulunamadı veya derlenemedi, geçiliyor..."

# Eğer setup, gui gibi ek alt sistem dosyaların varsa buraya ekleyebilirsin.
# Şimdilik hata vermemesi için eğer yoksa boş objeler oluşturulmasını engellemek adına 
# linker'a sadece elimizdeki kesin olan nesne dosyalarını vereceğiz.

echo "[3] Tüm nesne dosyaları linker.ld şablonuna göre birleştiriliyor..."
# Sadece mevcut nesne dosyalarını bağlayalım. kernel.o zaten stub'ları içeriyor.
if [ -f screen.o ]; then
    ld -m elf_i386 -T linker.ld -o kernel.bin boot.o kernel.o screen.o --no-warn-rwx-segments
else
    ld -m elf_i386 -T linker.ld -o kernel.bin boot.o kernel.o --no-warn-rwx-segments
fi

echo "[4] Boot edilebilir ISO imajı paketleniyor..."
# GRUB standartlarına uygun klasör yapısını kökten tertemiz inşa ediyoruz
mkdir -p iso_root/boot/grub

# Derlenen çekirdeği boot klasörünün altına taşıyoruz
cp kernel.bin iso_root/boot/kernel.bin

# Eksik olma ihtimaline karşı tertemiz bir grub.cfg oluşturuyoruz
cat << 'EOF' > iso_root/boot/grub/grub.cfg
set timeout=0
set default=0

menuentry "Sky Core OS / Wind OS" {
    multiboot /boot/kernel.bin
    boot
}
EOF

# xorriso hatasını ve GRUB2 "image too small" uyuşmazlığını aşmak için 
# standart grub-mkrescue komutunu doğrudan çağırıyoruz. 
# Bu komut arka planda xorriso'yu gerekli tüm GRUB modülleriyle doğru şekilde besler.
grub-mkrescue -o os_image.iso iso_root

echo "======================================================================"
echo "🎉 BAŞARILI: Sky Core OS ISO imajı (os_image.iso) başarıyla üretildi!"
echo "======================================================================"
