#include "printk.h"

void Start_Kernel(void)
{
    unsigned int *addr = (unsigned int *)0xffff800000a00000;
    int i;

    Pos.XResolution = 800;
    Pos.YResolution = 600;

    Pos.XPosition = Pos.YPosition = 0;

    Pos.XCharSize = 8;
    Pos.YCharSize = 16;

    Pos.FB_addr = (unsigned int *)0xffff800000a00000;
    Pos.EB_length = (Pos.XResolution * Pos.YResolution * 4);

    for (i = 0; i < 800 * 20; i++)
    {
        *((char *)addr + 0) = (char)0x00;
        *((char *)addr + 1) = (char)0x00;
        *((char *)addr + 2) = (char)0xff;
        *((char *)addr + 3) = (char)0x00;
        addr += 1;
    }

    for (i = 0; i < 800 * 20; i++)
    {
        *((char *)addr + 0) = (char)0x00;
        *((char *)addr + 1) = (char)0xff;
        *((char *)addr + 2) = (char)0x00;
        *((char *)addr + 3) = (char)0x00;
        addr += 1;
    }

    for (i = 0; i < 800 * 20; i++)
    {
        *((char *)addr + 0) = (char)0xff;
        *((char *)addr + 1) = (char)0x00;
        *((char *)addr + 2) = (char)0x00;
        *((char *)addr + 3) = (char)0x00;
        addr += 1;
    }

    for (i = 0; i < 800 * 20; i++)
    {
        *((char *)addr + 0) = (char)0xff;
        *((char *)addr + 1) = (char)0xff;
        *((char *)addr + 2) = (char)0xff;
        *((char *)addr + 3) = (char)0x00;
        addr += 1;
    }

    color_printk(YELLOW, BLACK, "Hello, World!\n");
    
    /*
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wdiv-by-zero"
        i = 1 / 0; // test interrupt
    #pragma GCC diagnostic pop
    */

    while (1)
        ;
}
