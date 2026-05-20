#ifndef KERNEL_H
#define KERNEL_H

/* Temel Veri Tipleri */
typedef unsigned int      u32;
typedef unsigned short    u16;
typedef unsigned char     u8;
typedef int               i32;
typedef signed char       i8;

/* GRUB Multiboot Bilgi Yapısı (kernel_main framebuffer okuması için) */
typedef struct {
    u32 flags;
    u32 mem_lower;
    u32 mem_upper;
    u32 boot_device;
    u32 cmdline;
    u32 mods_count;
    u32 mods_addr;
    u32 syms[4];
    u32 mmap_length;
    u32 mmap_addr;
    u32 drives_length;
    u32 drives_addr;
    u32 config_table;
    u32 boot_loader_name;
    u32 apm_table;
    u32 vbe_control_info;
    u32 vbe_mode_info;
    u16 vbe_mode;
    u16 vbe_interface_seg;
    u16 vbe_interface_off;
    u16 vbe_interface_len;
    u32 framebuffer_addr;
    u32 framebuffer_pitch;
    u32 framebuffer_width;
    u32 framebuffer_height;
    u8  framebuffer_bpp;
    u8  framebuffer_type;
} __attribute__((packed)) multiboot_info_t;

/* OS Durum Makinesi Tipleri */
typedef enum {
    STATE_SETUP_1_NAME,
    STATE_SETUP_2_REGION,
    STATE_SETUP_3_KEYBOARD,
    STATE_SETUP_4_NETWORK,
    STATE_SETUP_5_PRIVACY,
    STATE_SETUP_6_UPDATE,
    STATE_SETUP_7_FINISH,
    STATE_DESKTOP
} OS_State;

/* NOT: Eğer kernel.h içinde font8x8, inb, outb, mouse_btn, put_pixel, fill_rect 
   gibi fonksiyonların gövdeleri duruyorsa hepsini silin. kernel.c içinde tanımlandıkları için 
   burada durmaları redefinition (mükerrer tanımlama) hatasına yol açar.
*/

#endif
