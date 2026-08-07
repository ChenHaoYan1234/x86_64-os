#include <stdarg.h>
#include "printk.h"
#include "lib.h"

int skip_atoi(const char **s)
{
    int i = 0;

    while (is_digit(**s))
        i = i * 10 + *((*s)++) - '0';
    return i;
}

static char *number(char *str, long num, int base, int size, int precision, int type)
{
    char c, sign, tmp[50];
    const char *digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"; /* use upper-case letters */
    int i;

    if (type & SMALL)
        digits = "0123456789abcdefghijklmnopqrstuvwxyz"; /* use lower-case letters */

    if (type & LEFT)
        type &= ~ZEROPAD; /* zero-padding doesn't make sense with left-justification */

    if (base < 2 || base > 36)
        return NULL; /* base is invalid */

    c = (type & ZEROPAD) ? '0' : ' '; /* character to fill spaces with */

    sign = 0;
    if (type & SIGN && num < 0) /* handle sign */
    {
        sign = '-';
        num = -num;
    }
    else
        sign = (type & PLUS) ? '+' : (type & SPACE) ? ' '
                                                    : 0;
    if (sign)
        size--; /* make space for sign */

    if (type & SPECIAL) /* handle special prefixes */
    {
        if (base == 16)
            size -= 2;
        else if (base == 8)
            size--;
    }

    i = 0;
    if (num == 0)
        tmp[i++] = '0';
    else
        while (num != 0) /* convert number to string */
            tmp[i++] = digits[do_div(num, base)];

    if (i > precision)
        precision = i; /* precision doesn't make sense with less digits than precision */
    size -= precision; /* make space for digits */

    if (!(type & (ZEROPAD | LEFT))) /* fill spaces if not zero-padding and not left-justifying */
        while (size-- > 0)
            *str++ = ' ';

    if (sign) /* put sign in the right place */
        *str++ = sign;

    if (type & SPECIAL) /* add special prefixes */
    {
        if (base == 8)
            *str++ = '0';
        else if (base == 16)
        {
            *str++ = '0';
            *str++ = (type & SMALL) ? 'x' : 'X';
        }
    }

    if (!(type & LEFT))
        while (size-- > 0)
            *str++ = c; /* fill char before digits */

    while (i < precision--)
        *str++ = '0'; /* fill 0's before digits */

    while (i-- > 0)
        *str++ = tmp[i]; /* add the digits */

    while (size-- > 0)
        *str++ = ' '; /* fill spaces after digits */

    return str;
}

int vsprintf(char *buf, const char *fmt, va_list args)
{
    char *str, *s;
    int flags;
    int field_width;
    int precision;
    int len, i;
    int qualifier; /* 'h', 'l', 'L', or 'Z' for interger fields */

    for (str = buf; *fmt; fmt++)
    {
        if (*fmt != '%')
        {
            *str++ = *fmt;
            continue;
        }
        flags = 0;
    repeat:
        fmt++;
        switch (*fmt)
        {
        case '-':
            flags |= LEFT;
            goto repeat;
        case '+':
            flags |= PLUS;
            goto repeat;
        case ' ':
            flags |= SPACE;
            goto repeat;
        case '#':
            flags |= SPECIAL;
            goto repeat;
        case '0':
            flags |= ZEROPAD;
            goto repeat;
        }

        /* get field width */
        field_width = -1;
        if (is_digit(*fmt))
            field_width = skip_atoi(&fmt);
        else if (*fmt == '*')
        {
            fmt++;
            field_width = va_arg(args, int);
            if (field_width < 0)
            {
                field_width = -field_width;
                flags |= LEFT;
            }
        }

        /* get the precision */
        precision = -1;
        if (*fmt == '.')
        {
            fmt++;
            if (is_digit(*fmt))
                precision = skip_atoi(&fmt);
            else if (*fmt == '*')
            {
                fmt++;
                precision = va_arg(args, int);
            }
            if (precision < 0)
                precision = 0;
        }

        /* get the conversion qualifier */
        qualifier = -1;
        if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L' || *fmt == 'Z')
        {
            qualifier = *fmt;
            fmt++;
        }

        switch (*fmt)
        {
        case 'c':
            if (!(flags & LEFT))
                while (--field_width > 0)
                    *str++ = ' ';
            *str++ = (unsigned char)va_arg(args, int);
            while (++field_width > 0)
                *str++ = ' ';
            break;

        case 's':
            s = va_arg(args, char *);
            if (!s)
                s = '\0';
            len = strlen(s);
            if (precision < 0)
                len = precision;
            else if (len > precision)
                len = precision;

            if (!(flags & LEFT))
                while (len < field_width--)
                    *str++ = ' ';

            for (i = 0; i < len; i++)
                *str++ = *s++;
            while (len < field_width--)
                *str++ = ' ';
            break;

        case 'o':
            if (qualifier == 'l')
                str = number(str, va_arg(args, unsigned long), 8, field_width, precision, flags);
            else
                str = number(str, va_arg(args, unsigned int), 8, field_width, precision, flags);
            break;

        case 'p':
            if (field_width == -1)
            {
                field_width = 2 * sizeof(void *);
                flags |= ZEROPAD;
            }

            str = number(str, (unsigned long)va_arg(args, void *), 16, field_width, precision, flags);
            break;

        case 'x':
            flags |= SMALL;

        case 'X':
            if (qualifier == 'l')
                str = number(str, va_arg(args, unsigned long), 16, field_width, precision, flags);
            else
                str = number(str, va_arg(args, unsigned int), 16, field_width, precision, flags);
            break;

        case 'd':
        case 'i':
            flags |= SIGN;
        case 'u':
            if (qualifier == 'l')
                str = number(str, va_arg(args, unsigned long), 10, field_width, precision, flags);
            else
                str = number(str, va_arg(args, unsigned int), 10, field_width, precision, flags);
            break;

        case 'n':
            if (qualifier == 'l')
            {
                long *ip = va_arg(args, long *);
                *ip = str - buf;
            }
            else
            {
                int *ip = va_arg(args, int *);
                *ip = str - buf;
            }
            break;

        case '%':
            *str++ = '%';
            break;

        default:
            *str++ = '%';
            if (*fmt)
                *str++ = *fmt;
            else
                fmt--;
            break;
        }
    }
    *str = '\0';
    return str - buf;
}

void putchar(unsigned int *framebuffer, int x_size, int x, int y, unsigned int front_color, unsigned int background_color, unsigned char font)
{
    int i = 0, j = 0;
    unsigned int *addr = NULL;
    unsigned char *fontp = NULL;
    int testval = 0;
    fontp = font_ascii[font];

    for (i = 0; i < 16; i++)
    {
        addr = framebuffer + x_size * (y + i) + x;
        testval = 0x100;
        for (j = 0; j < 8; j++)
        {
            testval = testval >> 1;
            if (*fontp & testval)
                *addr = front_color;
            else
                *addr = background_color;
            addr++;
        }
        fontp++;
    }
}

int color_printk(unsigned int front_color, unsigned int background_color, const char *fmt, ...)
{
    int i = 0;
    int count;
    int line = 0;
    va_list args;
    va_start(args, fmt);

    i = vsprintf(buf, fmt, args);

    va_end(args);

    for (count = 0; count < i || line; count++)
    {
        // add \n \b \t
        if (line > 0)
        {
            count--;
            goto Label_tab;
        }
        if ((unsigned char)*(buf + count) == '\n')
        {
            Pos.y_position++;
            Pos.x_position = 0;
        }
        else if ((unsigned char)*(buf + count) == '\b')
        {
            Pos.x_position--;
            if (Pos.x_position < 0)
            {
                Pos.x_position = (Pos.x_resolution / Pos.x_char_size - 1);
                Pos.y_position--;
                if (Pos.y_position < 0)
                    Pos.y_position = (Pos.y_resolution / Pos.y_char_size - 1);
            }
            putchar(Pos.framebuffer, Pos.x_resolution, Pos.x_position * Pos.x_char_size, Pos.y_position * Pos.y_char_size, front_color, background_color, ' ');
        }
        else if ((unsigned char)*(buf + count) == '\t')
        {
            line = ((Pos.x_position + 8) & (~(8 - 1) - Pos.x_position));
        Label_tab:
            line--;
            putchar(Pos.framebuffer, Pos.x_resolution, Pos.x_position * Pos.x_char_size, Pos.y_position * Pos.y_char_size, front_color, background_color, ' ');
            Pos.x_position++;
        }
        else
        {
            putchar(Pos.framebuffer, Pos.x_resolution, Pos.x_position * Pos.x_char_size, Pos.y_position * Pos.y_char_size, front_color, background_color, (unsigned char)*(buf + count));
            Pos.x_position++;
        }

        if (Pos.x_position >= (Pos.x_resolution / Pos.x_char_size))
        {
            Pos.y_position++;
            Pos.x_position = 0;
        }
        if (Pos.y_position >= (Pos.y_resolution / Pos.y_char_size))
            Pos.y_position = 0;
    }
    return i;
}
