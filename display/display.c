#include "display/display.h"
#include <stdarg.h>
#include "include/string.h"

#define VGA_ADDR   0xB8000
#define VGA_WIDTH  80
#define VGA_HEIGHT 25
#define VGA_ATTR   0x0F

static volatile uint16_t* const vga = (uint16_t*)VGA_ADDR;
static int cursor_x = 0;
static int cursor_y = 0;

void kclean_display(void) {
    uint16_t blank = ((uint16_t)VGA_ATTR << 8) | ' ';
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga[i] = blank;
    }
    cursor_x = 0;
    cursor_y = 0;
}

static void kscroll_up(void) {
    uint16_t blank = ((uint16_t)VGA_ATTR << 8) | ' ';
    for (int row = 1; row < VGA_HEIGHT; row++) {
        for (int col = 0; col < VGA_WIDTH; col++) {
            vga[(row - 1) * VGA_WIDTH + col] = vga[row * VGA_WIDTH + col];
        }
    }
    for (int col = 0; col < VGA_WIDTH; col++) {
        vga[(VGA_HEIGHT - 1) * VGA_WIDTH + col] = blank;
    }
    cursor_y = VGA_HEIGHT - 1;
    if (cursor_x >= VGA_WIDTH) cursor_x = 0;
}

void kputc(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
            vga[cursor_y * VGA_WIDTH + cursor_x] = ((uint16_t)VGA_ATTR << 8) | ' ';
        } else if (cursor_y > 0) {
            cursor_y--;
            cursor_x = VGA_WIDTH - 1;
            vga[cursor_y * VGA_WIDTH + cursor_x] = ((uint16_t)VGA_ATTR << 8) | ' ';
        }
    } else if (c >= ' ') {
        if (cursor_x < 0) cursor_x = 0;
        if (cursor_y < 0) cursor_y = 0;
        if (cursor_x >= VGA_WIDTH) {
            cursor_x = 0;
            cursor_y++;
        }
        vga[cursor_y * VGA_WIDTH + cursor_x] = ((uint16_t)VGA_ATTR << 8) | (uint8_t)c;
        cursor_x++;
    }

    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }
    if (cursor_y >= VGA_HEIGHT) {
        kscroll_up();
    }
}

void kprint(const char* str) {
    if (!str) return;
    while (*str) kputc(*str++);
}

void kprint_num(int num) {
    if (num == 0) { kputc('0'); return; }

    unsigned int u;
    if (num < 0) {
        kputc('-');
        u = (unsigned int)(-(long long)num);
    } else {
        u = (unsigned int)num;
    }

    char buf[12];
    int i = 0;
    while (u > 0) {
        buf[i++] = '0' + (u % 10);
        u /= 10;
    }
    while (--i >= 0) kputc(buf[i]);
}

void kprint_hex(unsigned long num) {
    if (num == 0) { kputc('0'); return; }

    char buf[17];
    int i = 0;
    while (num) {
        unsigned int d = num & 0xF;
        buf[i++] = (d < 10) ? ('0' + d) : ('a' + d - 10);
        num >>= 4;
    }
    while (--i >= 0) kputc(buf[i]);
}

void kprint_ulong(unsigned long num) {
    if (num == 0) { kputc('0'); return; }

    char buf[21];
    int i = 0;
    while (num > 0) {
        buf[i++] = '0' + (num % 10);
        num /= 10;
    }
    while (--i >= 0) kputc(buf[i]);
}

void kprintf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    for (const char* p = fmt; *p; p++) {
        if (*p != '%') { kputc(*p); continue; }

        p++;
        if (!*p) break;

        switch (*p) {
            case 'c': kputc((char)va_arg(args, int)); break;
            case 's': kprint(va_arg(args, const char*) ?: "(null)"); break;
            case 'd': kprint_num(va_arg(args, int)); break;
            case 'x': kprint_hex(va_arg(args, unsigned int)); break;
            case 'l':
                p++;
                if (*p == 'x') kprint_hex(va_arg(args, unsigned long));
                else if (*p == 'u') kprint_ulong(va_arg(args, unsigned long));
                else if (*p == 'd') {
                    long v = va_arg(args, long);
                    if (v < 0) { kputc('-'); v = -v; }
                    kprint_ulong((unsigned long)v);
                }
                break;
            case '%': kputc('%'); break;
            default: kputc('%'); kputc(*p); break;
        }
    }
    va_end(args);
}
