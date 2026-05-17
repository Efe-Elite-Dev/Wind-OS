; =======================================================
; Wind OS / Sky-OS Hibrit Çekirdek Giriş Noktası
; =======================================================
MBOOT_PAGE_ALIGN    equ 1 << 0
MBOOT_MEM_INFO      equ 1 << 1
MBOOT_GRAPHICS      equ 1 << 2  ; Grafik modu desteği istiyoruz
MBOOT_MAGIC         equ 0x1BADB002
MBOOT_FLAGS         equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO | MBOOT_GRAPHICS
MBOOT_CHECKSUM      equ -(MBOOT_MAGIC + MBOOT_FLAGS)

section .multiboot
align 4
    dd MBOOT_MAGIC
    dd MBOOT_FLAGS
    dd MBOOT_CHECKSUM
    
    ; Grafik Bilgileri (GRUB bizim için 320x200x8 modunu açacak)
    dd 0
    dd 0
    dd 0
    dd 0
    dd 0        ; 0 = Linear Grafik Modu
    dd 320      ; Genişlik
    dd 200      ; Yükseklik
    dd 8        ; Renk Derinliği (256 Renk)

section .text
global _start
extern kernel_main

_start:
    mov esp, stack_space + 4096  ; 4KB Güvenli Yığın Alanı
    push eax                     ; Multiboot Sihirli Numarası
    push ebx                     ; Multiboot Bilgi Yapısı Adresi
    call kernel_main
    
.halt:
    cli
    hlt
    jmp .halt

section .bss
align 16
stack_space:
    resb 4096
