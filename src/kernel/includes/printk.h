#pragma once

#include <stdarg.h>
#include "font.h"

#define ZEROPAD 1  /* pad with zero */
#define SIGN 2     /* unsigned/signed long */
#define PLUS 4     /* show plus */
#define SPACE 8    /* space if plus */
#define LEFT 16    /* left justified */
#define SPECIAL 32 /* 0x */
#define SMALL 64   /* use 'abcdef' instead of 'ABCDEF*/

#define is_digit(c) ((c) >= '0' && (c) <= '9')

#define do_div(n, base) ({ \
int __res; \
__asm__("divq %%rcx" : "=a"(n), "=d"(__res) : "0"(n), "1"(0), "c"(base));\
__res; })

#define WHITE 0x00ffffff
#define BLACK 0x00000000
#define RED 0x00ff0000
#define ORANGE 0x00ff8000
#define YELLOW 0x00ffff00
#define GREEN 0x0000ff00
#define BLUE 0x000000ff
#define INDIGO 0x0000ffff
#define PURPLE 0x008000ff

extern unsigned char font_ascii[256][16];

char buf[4096] = {0};

struct Position
{
    int x_resolution;
    int y_resolution;

    int x_position;
    int y_position;

    int x_char_size;
    int y_char_size;

    unsigned int *framebuffer;
    unsigned int fb_length;
} Pos;

int skip_atoi(const char **s);

int vsprintf(char *buf, const char *fmt, va_list args);

void putchar(unsigned int *framebuffer, int x_size, int x, int y, unsigned int front_color, unsigned int background_color, unsigned char font);

int color_printk(unsigned int front_color, unsigned int background_color, const char *fmt, ...);
