#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdint.h>
#include <stdbool.h>

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768

extern uint32_t* GRAPHICS_FRAMEBUFFER;

#define gfx_framebuffer GRAPHICS_FRAMEBUFFER

extern int ai_hud_visible;

void force_graphics_hardware(void);

#endif
