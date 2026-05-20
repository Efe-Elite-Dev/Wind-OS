#ifndef KERNEL_H
#define KERNEL_H

/* 7 Kurulum Ekranı + Masaüstü */
typedef enum {
    STATE_SETUP_1_NAME = 0,
    STATE_SETUP_2_REGION,
    STATE_SETUP_3_KEYBOARD,
    STATE_SETUP_4_NETWORK,
    STATE_SETUP_5_PRIVACY,
    STATE_SETUP_6_CUSTOMIZE,
    STATE_SETUP_7_WELCOME,
    STATE_DESKTOP
} OS_State;

/* GRUB Multiboot bilgi yapısı — framebuffer dahil */
typedef struct {
    unsigned int   flags;
    unsigned int   mem_lower;
    unsigned int   mem_upper;
    unsigned int   boot_device;
    unsigned int   cmdline;
    unsigned int   mods_count;
    unsigned int   mods_addr;
    unsigned int   num;
    unsigned int   size;
    unsigned int   addr;
    unsigned int   shndx;
    unsigned int   mmap_length;
    unsigned int   mmap_addr;
    unsigned int   drives_length;
    unsigned int   drives_addr;
    unsigned int   config_table;
    unsigned int   boot_loader_name;
    unsigned int   apm_table;
    unsigned int   vbe_control_info;
    unsigned int   vbe_mode_info;
    unsigned short vbe_mode;
    unsigned short vbe_interface_seg;
    unsigned short vbe_interface_off;
    unsigned short vbe_interface_len;
    /* Linear Framebuffer (VBE) alanları */
    unsigned long long framebuffer_addr;   /* 64-bit fiziksel adres */
    unsigned int   framebuffer_pitch;      /* bayt/satır            */
    unsigned int   framebuffer_width;
    unsigned int   framebuffer_height;
    unsigned char  framebuffer_bpp;
    unsigned char  framebuffer_type;       /* 1=RGB  2=EGA text     */
    unsigned char  color_info[6];
} __attribute__((packed)) multiboot_info_t;

/* Kernel giriş noktası */
void kernel_main(multiboot_info_t *mbi);

#endif
