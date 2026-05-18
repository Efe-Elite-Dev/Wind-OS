#ifndef GUI_H
#define GUI_H

#include <stdint.h>

void draw_rect_pure(int x, int y, int w, int h, uint8_t color);
void draw_window_pure(int x, int y, int w, int h, const char* title);
void gui_init(void);

#endif
