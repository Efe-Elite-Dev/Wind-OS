#!/bin/bash
set -e

echo "[+] Boot kesiti derleniyor..."
nasm -f elf32 boot.asm -o boot.o

echo "[+] Saf kernel derleniyor..."
gcc -m32 -c kernel.c -o kernel.o -ffreestanding -O2 -Wall -Wextra -fno-exceptions

echo "[+] Nesne dosyalari baglaniyor (Linking)..."
ld -m elf_i386 -T linker.ld -o kernel.bin boot.o kernel.o

echo "[+] ISO dizin yapisi olusturuluyor..."
mkdir -p isodir/boot/grub
cp kernel.bin isodir/boot/kernel.bin
cp grub.cfg isodir/boot/grub/grub.cfg

echo "[+] Boot edilebilir ISO uretiliyor..."
grub-mkrescue -o windos.iso isodir

echo "[+] Temizlik yapiliyor..."
rm -rf boot.o kernel.o kernel.bin isodir

echo "[===] BASARILI: windos.iso hazir! VirtualBox veya QEMU ile atesleyebilirsin. [===]"
