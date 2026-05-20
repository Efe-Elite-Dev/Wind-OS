section .multiboot
align 4
    MAGIC    equ 0x1BADB002
    FLAGS    equ 0x00000003 ; Bit 0 (Hizala) + Bit 1 (Bellek Bilgisi), Grafik Modu KAPALI
    CHECKSUM equ -(MAGIC + FLAGS)

    dd MAGIC
    dd FLAGS
    dd CHECKSUM

section .bss
align 16
stack_bottom:
    resb 16384 ; Kernel için 16 KB güvenli yığın alanı
stack_top:

section .text
global _start
extern kernel_main

_start:
    cli                ; Donanım kesmelerini kapat
    mov esp, stack_top ; Stack pointer'ı ayarla

    call kernel_main   ; C koduna zıpla

.hang:
    hlt
    jmp .hang
