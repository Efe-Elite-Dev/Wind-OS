section .multiboot
align 4
    MULTIBOOT_MAGIC    equ 0x1BADB002
    ; Flagler: bit 0 (hizala) + bit 1 (bellek haritası) + bit 2 (grafik modu isteği)
    MULTIBOOT_FLAGS    equ 0x00000007 
    MULTIBOOT_CHECKSUM equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

    dd MULTIBOOT_MAGIC
    dd MULTIBOOT_FLAGS
    dd MULTIBOOT_CHECKSUM

    ; EĞER BIT 2 AKTİFSE BU ALANLAR TAM OLARAK BU SIRAYLA OLMALIDIR:
    dd 0          ; mode_type: 0 = Lineer Grafik Modu (VBE)
    dd 1024       ; width (Genişlik)
    dd 768        ; height (Yükseklik)
    dd 32         ; depth (Renk derinliği - bpp)
