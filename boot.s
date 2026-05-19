; boot.asm - Sky-Core OS Multiboot Header & Entry Point
MULTIBOOT_ALIGN     equ 1 << 0
MULTIBOOT_MEMINFO   equ 1 << 1
MULTIBOOT_GRAPHICS  equ 1 << 2  ; GRUB'a grafik modu açmasını söylüyoruz
MULTIBOOT_FLAGS     equ MULTIBOOT_ALIGN | MULTIBOOT_MEMINFO | MULTIBOOT_GRAPHICS
MULTIBOOT_MAGIC     equ 0x1BADB002
MULTIBOOT_CHECKSUM  equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

section .multiboot
align 4
    dd MULTIBOOT_MAGIC
    dd MULTIBOOT_FLAGS
    dd MULTIBOOT_CHECKSUM
    ; Grafik Ekran İstek Ayarları (800x600 32-bit renk)
    dd 0
    dd 800  ; Genişlik
    dd 600  ; Yükseklik
    dd 32   ; BPP

section .text
global _start
extern kernel_main

_start:
    cli
    mov esp, stack_top
    
    ; GRUB'ın bize verdiği boot parametrelerini C çekirdeğine argüman olarak paslıyoruz
    push ebx         ; Multiboot bilgi yapısının adresi (MBI)
    push eax         ; Sihirli numara (Magic Number)
    
    call kernel_main
    
.halt:
    hlt
    jmp .halt

section .bss
align 16
stack_bottom:
    resb 16384       ; 16KB Güvenli Stack Alanı
stack_top:
