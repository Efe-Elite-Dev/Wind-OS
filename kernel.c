/*
==============================================================================
WIND OS / SKY CORE OS v1.6 - Vortex Kernel STABLE
==============================================================================
*/

#include <stdint.h>
#include <stddef.h>

#define SW 1024
#define SH 768

/* ============================================================
 * GLOBALS
 * ============================================================ */

volatile uint32_t* GRAPHICS_FRAMEBUFFER = (uint32_t*)0xFD000000;
volatile int is_graphics_mode = 1;

static volatile int mouse_x = SW / 2;
static volatile int mouse_y = SH / 2;

/* ============================================================
 * STUBS / FIXES
 * ============================================================ */

/* LINKER HATASI ÇÖZÜMÜ */
void force_graphics_hardware(void)
{
    /* Şimdilik boş */
}

/* Diğer modüller isterse diye */
int ai_hud_visible = 0;

void screen_init(void) {}
void setup_init(void) {}

void setup_handle_input(uint8_t sc)
{
    (void)sc;
}

/* ============================================================
 * PORT IO
 * ============================================================ */

static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val)
{
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

/* ============================================================
 * SAFE DRAWING
 * ============================================================ */

static inline void put_px(int x, int y, uint32_t c)
{
    if ((unsigned)x >= SW || (unsigned)y >= SH)
        return;

    GRAPHICS_FRAMEBUFFER[y * SW + x] = c;
}

static void draw_rect(int x, int y, int w, int h, uint32_t c)
{
    if (w <= 0 || h <= 0)
        return;

    for (int yy = y; yy < y + h; yy++)
        for (int xx = x; xx < x + w; xx++)
            put_px(xx, yy, c);
}

static void draw_rect_outline(int x, int y, int w, int h, uint32_t c)
{
    if (w <= 0 || h <= 0)
        return;

    for (int i = x; i < x + w; i++) {
        put_px(i, y, c);
        put_px(i, y + h - 1, c);
    }

    for (int i = y; i < y + h; i++) {
        put_px(x, i, c);
        put_px(x + w - 1, i, c);
    }
}

/* ============================================================
 * GRADIENT
 * ============================================================ */

static void gradient_v(uint32_t top, uint32_t bot)
{
    for (int y = 0; y < SH; y++) {

        uint8_t r =
            (((top >> 16) & 0xFF) * (SH - y) +
             ((bot >> 16) & 0xFF) * y) / SH;

        uint8_t g =
            (((top >> 8) & 0xFF) * (SH - y) +
             ((bot >> 8) & 0xFF) * y) / SH;

        uint8_t b =
            (((top) & 0xFF) * (SH - y) +
             ((bot) & 0xFF) * y) / SH;

        uint32_t col =
            0xFF000000 |
            (r << 16) |
            (g << 8) |
            b;

        for (int x = 0; x < SW; x++)
            GRAPHICS_FRAMEBUFFER[y * SW + x] = col;
    }
}

/* ============================================================
 * CURSOR
 * ============================================================ */

static void draw_cursor(int x, int y)
{
    for (int i = 0; i < 12; i++) {

        for (int j = 0; j <= i; j++) {

            uint32_t col =
                (j == 0 || j == i)
                ? 0xFF000000
                : 0xFFFFFFFF;

            put_px(x + j, y + i, col);
        }
    }
}

/* ============================================================
 * SIMPLE DESKTOP
 * ============================================================ */

static void render_desktop(void)
{
    gradient_v(0xFF1A0F2E, 0xFF0D0B18);

    /* Üst bar */
    draw_rect(0, 0, SW, 38, 0xAA14102A);

    /* Sol panel */
    draw_rect(20, 60, 300, 120, 0xAA211C38);
    draw_rect_outline(20, 60, 300, 120, 0xFF00E5FF);

    /* Alt dock */
    draw_rect(20, SH - 72, 700, 56, 0xAA211C38);
    draw_rect_outline(20, SH - 72, 700, 56, 0xFF00E5FF);

    /* Basit pencere */
    draw_rect(340, 180, 340, 220, 0xFF111122);
    draw_rect_outline(340, 180, 340, 220, 0xFF00E5FF);

    draw_cursor(mouse_x, mouse_y);
}

/* ============================================================
 * MOUSE DRIVER
 * ============================================================ */

static void mouse_wait_in(void)
{
    uint32_t t = 100000;

    while (t--) {
        if (inb(0x64) & 1)
            return;
    }
}

static void mouse_wait_out(void)
{
    uint32_t t = 100000;

    while (t--) {
        if (!(inb(0x64) & 2))
            return;
    }
}

static void mouse_send(uint8_t data)
{
    mouse_wait_out();
    outb(0x64, 0xD4);

    mouse_wait_out();
    outb(0x60, data);

    mouse_wait_in();
    inb(0x60);
}

void mouse_init(void)
{
    mouse_wait_out();
    outb(0x64, 0xA8);

    mouse_wait_out();
    outb(0x64, 0x20);

    mouse_wait_in();

    uint8_t status = inb(0x60);
    status |= 2;

    mouse_wait_out();
    outb(0x64, 0x60);

    mouse_wait_out();
    outb(0x60, status);

    mouse_send(0xF6);
    mouse_send(0xF4);
}

static void handle_mouse(void)
{
    static uint8_t cycle = 0;
    static uint8_t packet[3];

    uint8_t status = inb(0x64);

    if (!(status & 1))
        return;

    if (!(status & 0x20)) {
        inb(0x60);
        return;
    }

    packet[cycle++] = inb(0x60);

    if (cycle < 3)
        return;

    cycle = 0;

    if (!(packet[0] & 0x08))
        return;

    mouse_x += ((int8_t)packet[1]) / 2;
    mouse_y -= ((int8_t)packet[2]) / 2;

    if (mouse_x < 0) mouse_x = 0;
    if (mouse_y < 0) mouse_y = 0;

    if (mouse_x >= SW)
        mouse_x = SW - 1;

    if (mouse_y >= SH)
        mouse_y = SH - 1;
}

/* ============================================================
 * PIC
 * ============================================================ */

void idt_init(void)
{
    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    outb(0x21, 0x20);
    outb(0xA1, 0x28);

    outb(0x21, 0x04);
    outb(0xA1, 0x02);

    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    outb(0x21, 0x00);
    outb(0xA1, 0x00);
}

/* ============================================================
 * MAIN
 * ============================================================ */

void kernel_main(void* mboot_ptr, uint32_t magic)
{
    (void)magic;

    if (mboot_ptr) {

        uint32_t flags = *(uint32_t*)mboot_ptr;

        if (flags & (1 << 11)) {

            uint32_t fb_addr =
                ((uint32_t*)mboot_ptr)[22];

            if (fb_addr)
                GRAPHICS_FRAMEBUFFER =
                    (uint32_t*)fb_addr;
        }
    }

    force_graphics_hardware();

    idt_init();

    mouse_init();

    render_desktop();

    while (1) {

        handle_mouse();

        render_desktop();

        __asm__ volatile("hlt");
    }
}
