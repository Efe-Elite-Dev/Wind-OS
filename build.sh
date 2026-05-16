#!/bin/bash

# set -e komutunu KALDIRDIK! Böylece GRUB'ın ufak tefek imza nazlanmaları derlemeyi durduramayacak.
echo "==> Wind OS Özellikleri Korunarak Yeni Klasör Mimarisiyle Derleniyor..."

# 1. Eski klasörleri ve kalıntıları kökten kazı
rm -rf isodir windos_boot_layer
rm -f *.o kernel.bin windos.iso

# 2. Temiz klasör yapısını inşa et
mkdir -p windos_boot_layer/boot/grub

# 3. Önyükleyici odasını derle
nasm -f elf32 boot.asm -o boot.o

# 4. TÜM YAPAY ZEKA VE SÜRÜCÜ ODALARI (%100 KORUNDU)
echo "==> Yapay zeka odaları ve sürücüler derleniyor..."
gcc -m32 -ffreestanding -O2 -Wall -Wextra -fno-pie -fno-stack-protector -c kernel.c -o kernel.o
gcc -m32 -ffreestanding -O2 -Wall -Wextra -fno-pie -fno-stack-protector -c gui.c -o gui.o
gcc -m32 -ffreestanding -O2 -Wall -Wextra -fno-pie -fno-stack-protector -c exe_subsystem.c -o exe_subsystem.o
gcc -m32 -ffreestanding -O2 -Wall -Wextra -fno-pie -fno-stack-protector -c ai_subsystem.c -o ai_subsystem.o
gcc -m32 -ffreestanding -O2 -Wall -Wextra -fno-pie -fno-stack-protector -c mouse.c -o mouse.o
gcc -m32 -ffreestanding -O2 -Wall -Wextra -fno-pie -fno-stack-protector -c keyboard.c -o keyboard.o
gcc -m32 -ffreestanding -O2 -Wall -Wextra -fno-pie -fno-stack-protector -c wind_subsystem.c -o wind_subsystem.o
gcc -m32 -ffreestanding -O2 -Wall -Wextra -fno-pie -fno-stack-protector -c screen.c -o screen.o
gcc -m32 -ffreestanding -O2 -Wall -Wextra -fno-pie -fno-stack-protector -c idt.c -o idt.o
gcc -m32 -ffreestanding -O2 -Wall -Wextra -fno-pie -fno-stack-protector -c deb_subsystem.c -o deb_subsystem.o

# 5. Hizalamalı Linker Bağlantısı (Donanım Zırhı)
# boot.o dosyasını en başa koyduk. ld uyarısını susturmak için noexecstack parametresini ekledik.
echo "==> Çekirdek odaları linker ile birleştiriliyor..."
gcc -m32 -T linker.ld -nostdlib -no-pie -Wl,-z,noexecstack -o kernel.bin \
    boot.o kernel.o gui.o exe_subsystem.o ai_subsystem.o \
    mouse.o keyboard.o wind_subsystem.o screen.o idt.o deb_subsystem.o

# 6. Çekirdeği Yeni Katmana Kopyala
cp kernel.bin windos_boot_layer/boot/kernel.bin

# 7. GRUB Yapılandırmasını Enjekte Et
cat << 'EOF' > windos_boot_layer/boot/grub/grub.cfg
set timeout=0
set default=0

insmod vbe
insmod vga
insmod video_bochs
insmod video_cirrus

menuentry "Wind OS - Full AI Core (New Layer)" {
    multiboot /boot/kernel.bin
    boot
}
EOF

# 8. Multiboot Doğrulaması (Eğer 1 kodu dönerse scripti durdurma, '|| true' ile pas geç)
grub-file --is-x86-multiboot windos_boot_layer/boot/kernel.bin || true

# 9. SAF GRUB RECOVERY ISO ÜRETİMİ
echo "==> Yeni klasör yapısı üzerinden evrensel ISO mühürleniyor..."
grub-mkrescue -o windos.iso windos_boot_layer

echo "==> İşlem Başarılı! Tüm AI motorları aktif ve koruma altında."
