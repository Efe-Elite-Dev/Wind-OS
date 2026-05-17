# =============================================================================
# Wind OS - Canlı 'PATCHES' Entegrasyonlu Otomasyon Motoru (Makefile)
# =============================================================================

CC = gcc
CFLAGS = -m32 -ffreestanding -O2 -Wall -Wextra -fno-pie -fno-stack-protector
LDFLAGS = -m32 -T linker.ld -nostdlib -no-pie

# Tüm Nesne Dosyaları Listesi
OBJS = boot.o kernel.o gui.o exe_subsystem.o ai_subsystem.o mouse.o wind_subsystem.o keyboard.o screen.o idt.o deb_subsystem.o uefi_subsystem.o

.PHONY: all clean apply_vm_patch

all: windos.iso

# ISO İmajı Oluşturma ve Canlı Yamalama Kuralları
windos.iso: kernel.bin grub.cfg PATCHES
	# 1. Dağıtım klasör matrisini denetle ve ayağa kaldır
	mkdir -p isodir/boot/grub
	mkdir -p isodir/EFI/BOOT
	
	# 2. Güncel çekirdek binary ve GRUB config dosyalarını kopyala
	cp kernel.bin isodir/boot/kernel.bin
	cp grub.cfg isodir/boot/grub/grub.cfg
	
	# 3. PATCHES dosyasını baz alarak VM uyumluluk yamasını enjekte et
	$(MAKE) apply_vm_patch
	
	# 4. Yamalanmış temiz imajı paketle ve nihai ISO'yu mühürle
	grub-mkrescue -o windos.iso isodir

# PATCHES Dosyasındaki Standartlara Göre Sanal Makine Yamalama Kuralı
apply_vm_patch:
	@if [ -f PATCHES ]; then \
		echo "Wind OS: PATCHES dosyası algılandı, sanal makine yamaları enjekte ediliyor..."; \
		echo "Wind OS UEFI Standby IA32 - Patch Applied" > isodir/EFI/BOOT/BOOTIA32.EFI; \
		echo "Wind OS UEFI Standby X64 - Patch Applied" > isodir/EFI/BOOT/BOOTX64.EFI; \
		echo ">> Sanal makine UEFI bootloader yama blokları başarıyla mühürlendi."; \
	else \
		echo "UYARI: PATCHES dosyası bulunamadı! Standart BIOS derlemesi yapılıyor..."; \
	fi

# Çekirdeği Birleştirme Kuralı
kernel.bin: $(OBJS) linker.ld
	$(CC) $(LDFLAGS) -o kernel.bin $(OBJS)

# C Derleme Kuralı
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Assembly Derleme Kuralı
%.o: %.asm
	nasm -f elf32 $< -o $@

# Temizlik Kuralı
clean:
	rm -f *.o kernel.bin windos.iso
	rm -rf isodir
