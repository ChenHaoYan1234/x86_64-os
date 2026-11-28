#pragma once

#include "lib.h"
#include "memory.h"

#define VBE_INFO_PHYS 0x8000UL
#define VBE_MODE_INFO_PHYS 0x8200UL
#define VBE_MODE_SCAN_MAX 256

#define VBE_MODE_ATTR_LFB 0x80
#define VBE_MEMORY_MODEL_DIRECT_COLOR 6

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
};

struct vbe_info_block
{
    char signature[4];
    unsigned short version;
    unsigned short oem_string_ptr[2];
    unsigned int capabilities;
    unsigned short video_mode_ptr[2];
    unsigned short total_memory;
} __attribute__((packed));

struct vbe_mode_info
{
    unsigned short mode_attributes;
    unsigned char win_a_attributes;
    unsigned char win_b_attributes;
    unsigned short win_granularity;
    unsigned short win_size;
    unsigned short win_a_segment;
    unsigned short win_b_segment;
    unsigned int win_func_ptr;
    unsigned short bytes_per_scanline;
    unsigned short x_resolution;
    unsigned short y_resolution;
    unsigned char x_char_size;
    unsigned char y_char_size;
    unsigned char number_of_planes;
    unsigned char bits_per_pixel;
    unsigned char number_of_banks;
    unsigned char memory_model;
    unsigned char bank_size;
    unsigned char number_of_image_pages;
    unsigned char reserved0;
    unsigned char red_mask_size;
    unsigned char red_field_position;
    unsigned char green_mask_size;
    unsigned char green_field_position;
    unsigned char blue_mask_size;
    unsigned char blue_field_position;
    unsigned char rsvd_mask_size;
    unsigned char rsvd_field_position;
    unsigned char direct_color_mode_info;
    unsigned int phys_base_ptr;
    unsigned int off_screen_mem_offset;
    unsigned short off_screen_mem_size;
    unsigned char reserved1[206];
} __attribute__((packed));

_Static_assert(sizeof(struct vbe_mode_info) == 0x100, "VBE mode info block must be 256 bytes");

extern struct Position Pos;

static inline int vbe_present(void)
{
    struct vbe_info_block *vbe = (struct vbe_info_block *)(PAGE_OFFSET + VBE_INFO_PHYS);

    return vbe->signature[0] == 'V' && vbe->signature[1] == 'E' &&
           vbe->signature[2] == 'S' && vbe->signature[3] == 'A';
}

static inline unsigned long find_lfb_base(void)
{
    struct vbe_mode_info *mode = (struct vbe_mode_info *)(PAGE_OFFSET + VBE_MODE_INFO_PHYS);
    unsigned long i;

    if (!vbe_present())
        return 0xe0000000UL;

    for (i = 0; i < VBE_MODE_SCAN_MAX; i++, mode++)
    {
        if (!(mode->mode_attributes & VBE_MODE_ATTR_LFB))
            continue;
        if (mode->x_resolution != 800 || mode->y_resolution != 600 ||
            mode->number_of_planes != 1 || mode->bits_per_pixel != 32 ||
            mode->memory_model != VBE_MEMORY_MODEL_DIRECT_COLOR ||
            mode->bytes_per_scanline < 800 * 4 || mode->phys_base_ptr == 0)
            continue;

        return mode->phys_base_ptr;
    }

    return 0xe0000000UL; /* Bochs VBE LFB */
}

static inline void init_framebuffer(void)
{
    unsigned long phys = find_lfb_base();
    unsigned long cr3;

    __PDE[5] = (phys & PAGE_2M_MASK) | 0x83; /* present, writable, 2MB page */

    __asm__ __volatile__("movq %%cr3, %0" : "=r"(cr3));
    __asm__ __volatile__("movq %0, %%cr3" : : "r"(cr3) : "memory");

    Pos.framebuffer = (unsigned int *)(0xffff800000a00000UL + (phys & (PAGE_2M_SIZE - 1)));
}
