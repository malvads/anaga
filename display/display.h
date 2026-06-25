#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

void kclean_display(void);
void kputc(char c);
void kprint(const char* str);
void kprint_num(int num);
void kprint_hex(unsigned long num);
void kprint_ulong(unsigned long num);
void kprintf(const char* fmt, ...);

#endif