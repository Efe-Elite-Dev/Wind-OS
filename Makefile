# Derleyici ve Bayrak Tanımlamaları
CC = gcc
CFLAGS = -m32 -ffreestanding -O2 -Wall -Wextra -fno-pie -fno-stack-protector
LDFLAGS = -m32 -T linker.ld -nostdlib -no-pie

# Nesne Dosyaları Listesi (Tüm Modüller Tam Eşleşme)
OBJS = boot.o kernel.o gui.o exe_subsystem.o ai_subsystem.o mouse.o wind_subsystem.o keyboard.o screen.o idt.o deb_subsystem.o

# Sahte Hedef Tanımlamaları (Dosya isimleriyle çakışmayı engeller)
.PHONY: all clean

# Ana Hedef
all: windos.iso

# ISO İmajı Oluşturma Motoru
windos.iso: kernel.bin grub.cfg
	mkdir -p isodir/boot/grub
	cp kernel.bin isodir/boot/kernel.bin
	cp grub.cfg isodir/boot/grub/grub.cfg
	grub-mkrescue -o windos.iso isodir

# Çekirdeği (Kernel) Birleştirme Kuralı
kernel.bin: $(OBJS) linker.ld
	$(CC) $(LDFLAGS) -o kernel.bin $(OBJS)

# C Kaynak Dosyalarını Derleme Kuralı
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Assembly Giriş Dosyasını Derleme Kuralı
%.o: %.asm
	nasm -f elf32 $< -o $@

# Temizlik Kuralı
clean:
	rm -f *.o kernel.bin windos.iso
	rm -rf isodir
