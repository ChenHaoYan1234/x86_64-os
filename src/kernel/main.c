#include "gate.h"
#include "interrupt.h"
#include "lib.h"
#include "memory.h"
#include "printk.h"
#include "trap.h"

struct GlobalMemoryDescriptor memory_management_struct = {0};

extern char _text;
extern char _etext;
extern char _edata;
extern char _end;

void Start_Kernel(void)
{
    unsigned int *addr = (unsigned int *)0xffff800000a00000;
    unsigned int i;

    Pos.x_resolution = 800;
    Pos.y_resolution = 600;

    Pos.x_position = Pos.y_position = 0;

    Pos.x_char_size = 8;
    Pos.y_char_size = 16;

    Pos.framebuffer = (unsigned int *)0xffff800000a00000;
    Pos.fb_length = (Pos.x_resolution * Pos.y_resolution * 4 + PAGE_4K_SIZE - 1) & PAGE_4K_MASK;

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

    load_TR(8);

    set_tss64(0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00,
              0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00, 0xffff800000007c00);

    sys_vector_init();

    // #pragma GCC diagnostic push
    // #pragma GCC diagnostic ignored "-Wdiv-by-zero"
    //     i = 1 / 0; // test interrupt
    // #pragma GCC diagnostic pop

    memory_management_struct.start_code = (unsigned long)&_text;
    memory_management_struct.end_code = (unsigned long)&_etext;
    memory_management_struct.end_data = (unsigned long)&_edata;
    memory_management_struct.end_brk = (unsigned long)&_end;

    color_printk(RED, BLACK, "memory init\n");
    init_memory();

    color_printk(RED,BLACK, "interrupt init\n");
    init_interrupt();

    while (1)
        ;
}
