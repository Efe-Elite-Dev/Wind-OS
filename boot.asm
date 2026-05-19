section .multiboot
align 4
    MULTIBOOT_MAGIC    equ 0x1BADB002
    MULTIBOOT_FLAGS    equ 0x00000003 ; SADECE METİN MODU (0x07 olan grafik modunu iptal ettik)
    MULTIBOOT_CHECKSUM equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

    dd MULTIBOOT_MAGIC
    dd MULTIBOOT_FLAGS
    dd MULTIBOOT_CHECKSUM

    ; EĞER BURADA ALT KISIMDA "dd 1024", "dd 768" GİBİ SATIRLAR VARSA ONLARI TAMAMEN SİL.
    ; Text modunda onlara ihtiyacımız yok, silmezsen GRUB yine kafayı yer.

; --- STACK ALANI (Aynı kalacak) ---
section .bss
align 16
stack_bottom:
    resb 16384 
stack_top:

; --- GİRİŞ NOKTASI (Aynı kalacak) ---
section .text
global _start
extern kernel_main 

_start:
    cli 
    mov esp, stack_top 

    push ebx 
    push eax 

    call kernel_main

.hang:
    hlt 
    jmp .hang
