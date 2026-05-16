#include "wind_subsystem.h"

/* kernel.c içindeki pointer mimarisini extern olarak bağlıyoruz */
extern uint32_t* back_buffer;
int setup_stage = 0;
extern int central_ai_prediction_level;

void draw_pixel_pure(int x, int y, uint32_t color) {
    if (x >= 0 && x < 800 && y >= 0 && y < 600) {
        back_buffer[y * 800 + x] = color;
    }
}

void draw_filled_rect(int x, int y, int width, int height, uint32_t color) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            draw_pixel_pure(x + j, y + i, color);
        }
    }
}

void draw_rect_outline(int x, int y, int width, int height, uint32_t color) {
    for (int i = 0; i < width; i++) {
        draw_pixel_pure(x + i, y, color);
        draw_pixel_pure(x + i, y + height - 1, color);
    }
    for (int i = 0; i < height; i++) {
        draw_pixel_pure(x, y + i, color);
        draw_pixel_pure(x + width - 1, y + i, color);
    }
}

void draw_custom_window(int x, int y, int width, int height, const char* title, uint32_t body_color) {
    draw_filled_rect(x, y, width, height, body_color);
    draw_filled_rect(x, y, width, 24, 0x001A4473); 
    draw_rect_outline(x, y, width, height, 0x00FFFFFF); 

    int btn_w = 90;
    int btn_h = 25;
    int btn_x = x + width - btn_w - 15;
    int btn_y = y + height - btn_h - 15;
    draw_filled_rect(btn_x, btn_y, btn_w, btn_h, 0x000A3A6B); 
    draw_rect_outline(btn_x, btn_y, btn_w, btn_h, 0x00FFFFFF); 
}

void run_exe_subsystem(void) {
    int win_x = 150;
    int win_y = 100;
    int win_w = 500;
    int win_h = 380;

    if (setup_stage == 0) {
        draw_custom_window(win_x, win_y, win_w, win_h, "Wind OS - AI Setup Step 1", 0x00CCCCCC);
        uint32_t ai_status_color = (central_ai_prediction_level == 2) ? 0x00FF0000 : 0x0000FF00;
        draw_filled_rect(win_x + 40, win_y + win_h - 60, win_w - 80, 4, ai_status_color);
        
    } else if (setup_stage == 1) {
        draw_custom_window(win_x, win_y, win_w, win_h, "Wind OS - License Agreement Step 2", 0x00DDDDDD);
        int box_x = win_x + 20;
        int box_y = win_y + 50;
        int box_w = win_w - 40;
        int box_h = win_h - 110;
        draw_filled_rect(box_x, box_y, box_w, box_h, 0x00FFFFFF); 
        draw_rect_outline(box_x, box_y, box_w, box_h, 0x00888888); 
        draw_filled_rect(box_x + 15, box_y + 15, 180, 12, 0x001A4473);
        draw_filled_rect(box_x + 15, box_y + 35, 250, 10, 0x0000AA00); 
        draw_filled_rect(box_x + 15, box_y + 65, box_w - 30, 6, 0x00888888);
        draw_filled_rect(box_x + 15, box_y + 80, box_w - 30, 6, 0x00888888);
        draw_filled_rect(box_x + 15, box_y + 95, box_w - 80, 6, 0x00888888);
        
    } else if (setup_stage == 2) {
        draw_custom_window(win_x, win_y, win_w, win_h, "Hardware Calibration Step 3", 0x00BBBBBB);
        draw_filled_rect(win_x + 50, win_y + 150, 400, 8, 0x0000FF00);
        
    } else if (setup_stage == 3) {
        draw_custom_window(win_x, win_y, win_w, win_h, "Finalizing OS Setup Step 4", 0x00AAAAAA);
        draw_filled_rect(win_x + 50, win_y + 200, 400, 12, 0x000A3A6B);
    }
}
