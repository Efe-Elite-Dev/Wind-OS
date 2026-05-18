#!/bin/bash
set -e # Herhangi bir adım hata verirse derlemeyi durdur

echo "======================================================================"
echo "🚀 SKY CORE OS / WIND OS MEGA BUILD ENGINE"
echo "======================================================================"

echo "[-] Eski derleme kalıntıları temizleniyor..."
rm -rf *.o kernel.bin os_image.iso iso_root/boot/kernel.bin

echo "[1] Çekirdek önyükleme mekanizması derleniyor (boot.asm)..."
nasm -f elf32 boot.asm -o boot.o

echo "[2] Çekirdek ve tüm alt sistemler derleniyor..."
gcc -m32 -c kernel.c -o kernel.o -ffreestanding -O2 -fno-exceptions -fno-stack-protector
gcc -m32 -c setup.c -o setup.o -ffreestanding -O2 -fno-exceptions -fno-stack-protector
gcc -m32 -c setup_ui.c -o setup_ui.o -ffreestanding -O2 -fno-exceptions -fno-stack-protector
gcc -m32 -c gui.c -o gui.o -ffreestanding -O2 -fno-exceptions -fno-stack-protector
gcc -m32 -c screen.c -o screen.o -ffreestanding -O2 -fno-exceptions -fno-stack-protector
gcc -m32 -c mouse.c -o mouse.o -ffreestanding -O2 -fno-exceptions -fno-stack-protector
gcc -m32 -c keyboard.c -o keyboard.o -ffreestanding -O2 -fno-exceptions -fno-stack-protector
gcc -m32 -c idt.c -o idt.o -ffreestanding -O2 -fno-exceptions -fno-stack-protector
gcc -m32 -c wind_subsystem.c -o wind_subsystem.o -ffreestanding -O2 -fno-exceptions -fno-stack-protector
gcc -m32 -c exe_subsystem.c -o exe_subsystem.o -ffreestanding -O2 -fno-exceptions -fno-stack-protector
gcc -m32 -c ai_subsystem.c -o ai_subsystem.o -ffreestanding -O2 -fno-exceptions -fno-stack-protector
gcc -m32 -c deb_subsystem.c -o deb_subsystem.o -ffreestanding -O2 -fno-exceptions -fno-stack-protector

# 🔥 YENİ EKLENEN DONANIM VE HATA KORUMA DOSYALARI
gcc -m32 -c vga_force.c -o vga_force.o -ffreestanding -O2 -fno-exceptions -fno-stack-protector
gcc -m32 -c kerror.c -o kerror.o -ffreestanding -O2 -fno-exceptions -fno-stack-protector

echo "[3] Tüm nesne dosyaları linker.ld şablonuna göre birleştiriliyor..."
ld -m elf_i386 -T linker.ld -o kernel.bin \
    boot.o kernel.o setup.o setup_ui.o gui.o screen.o mouse.o keyboard.o idt.o \
    wind_subsystem.o exe_subsystem.o ai_subsystem.o deb_subsystem.o \
    vga_force.o kerror.o

echo "[4] Boot edilebilir ISO imajı paketleniyor..."
mkdir -p iso_root/boot/grub
cp kernel.bin iso_root/boot/kernel.bin
cp grub.cfg iso_root/boot/grub/grub.cfg

# Güvenli ve bağımsız xorriso paketlemesi
xorriso -as mkisofs -R -b boot/grub/i386-pc/eltorito.img \
    -no-emul-boot -boot-load-size 4 -boot-info-table \
    --grub2-boot-info --graft-points \
    -o os_image.iso \
    /boot/grub/i386-pc/eltorito.img=/usr/lib/grub/i386-pc/boot_hybrid.img \
    iso_root

echo "[+] BAŞARILI: os_image.iso hazır aga!"
