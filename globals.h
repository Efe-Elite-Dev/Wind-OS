#ifndef GLOBALS_H
#define GLOBALS_H

typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;
typedef int            bool;

#define true  1
#define false 0
#define NULL  ((void*)0)

#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 600
#define GRAPHICS_FRAMEBUFFER 0xA0000

typedef enum {
    STATE_WELCOME,
    STATE_LOCATION,
    STATE_COMPLETING,
    STATE_DESKTOP
} SystemState;

typedef struct {
    char computer_name[64];
    char location[64];
    char timezone[16];
    bool wifi_connected;
} SetupData;

// Sadece extern olarak bırakıyoruz, bellekte yer ayırmıyoruz
extern SystemState current_state;
extern bool ai_hud_visible;
extern SetupData os_setup_data;

#endif
