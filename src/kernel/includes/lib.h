#pragma once

#define NULL 0

#define sti() __asm__ __volatile__("sti \n\t" ::: "memory")

#define container_of(ptr, type, member)                                     \
    ({                                                                      \
        typeof(((type *)0)->member) *p = (ptr);                             \
        (type *)((unsigned long)p - (unsigned long)&(((type *)0)->member)); \
    })

static inline void io_out8(unsigned short port, unsigned char data)
{
    __asm__ __volatile__(
        "outb %0, %%dx \n\t"
        "mfence \n\t"
        :
        : "a"(data), "d"(port)
        : "memory");
}

static inline unsigned char io_in8(unsigned short port)
{
    unsigned char ret = 0;
    __asm__ __volatile__(
        "inb %%dx, %0 \n\t"
        "mfence \n\t"
        : "=a"(ret)
        : "d"(port)
        : "memory");
    return ret;
}

static inline int strlen(char *string)
{
    register int __res;
    __asm__ __volatile__(
        "cld \n\t"
        "repne \n\t"
        "scasb \n\t"
        "notl %0 \n\t"
        "decl %0 \n\t"
        : "=c"(__res)
        : "D"(string), "a"(0), "0"(0xffffffff)
        :);
    return __res;
}

static inline void *memset(void *dest, unsigned char val, unsigned long size)
{
    int d0, d1;
    unsigned long tmp = val * 0x0101010101010101UL;
    __asm__ __volatile__(
        "cld \n\t"
        "rep \n\t"
        "stosq \n\t"
        "testb $4, %b3 \n\t"
        "je 1f \n\t"
        "stosl \n\t"
        "1:\t testb $2, %b3 \n\t"
        "je 2f \n\t"
        "stosw \n\t"
        "2:\t testb $1, %b3 \n\t"
        "je 3f \n\t"
        "stosb \n\t"
        "3: \n\t"
        : "=&c"(d0), "=&D"(d1)
        : "a"(tmp), "q"(size), "0"(size / 8), "1"(dest)
        : "memory");
    return dest;
}

static void *memcpy(void *dest, void *src, unsigned long size)
{
    int d0, d1, d2;
    __asm__ __volatile__(
        "cld \n\t"
        "rep \n\t"
        "movsq \n\t"
        "testb $4, %b4 \n\t"
        "je 1f \n\t"
        "movsl \n\t"
        "1: \n\t"
        "testb $2, %b4 \n\t"
        "je 2f \n\t"
        "movsw \n\t"
        "2: \n\t"
        "testb $1, %b4 \n\t"
        "je 3f \n\t"
        "movsb \n\t"
        "3: \n\t"
        : "=&c"(d0), "=&D"(d1), "=&S"(d2)
        : "0"(size / 8), "q"(size), "1"(dest), "2"(src)
        : "memory");
    return dest;
};
