#pragma once

// 8 bytes per cell
#define PTRS_PER_PAGE 512

#define PAGE_OFFSET ((unsigned long)0xffff800000000000)

#define PAGE_GDT_SHIFT 39
#define PAGE_1G_SHIFT 30
#define PAGE_2M_SHIFT 21
#define PAGE_4K_SHIFT 12

#define PAGE_2M_SIZE (1UL << PAGE_2M_SHIFT)
#define PAGE_4K_SIZE (1UL << PAGE_4K_SHIFT)

#define PAGE_2M_MASK (~(PAGE_2M_SIZE - 1))
#define PAGE_4K_MASK (~(PAGE_4K_SIZE - 1))

#define PAGE_2M_ALIGN(addr) (((unsigned long)(addr) + PAGE_2M_SIZE - 1) & PAGE_2M_MASK)
#define PAGE_4K_ALIGN(addr) (((unsigned long)(addr) + PAGE_4K_SIZE - 1) & PAGE_4K_MASK)

#define VIRT_TO_PHYS(addr) ((unsigned long)(addr) - PAGE_OFFSET)
#define PHYS_TO_VIRT(addr) ((unsigned long *)((unsigned long)(addr) + PAGE_OFFSET))

// bit 63 Execution Disable
#define PAGE_XD (unsigned long)0x1000000000000000

// bit 12 Page Attrible Table
#define PAGE_PAT (unsigned long)0x1000

// bit 8 Global Page: 1 = global, 0 = part
#define PAGE_GLOBAL (unsigned long)0x0100

// bit 7 Page Size: 1 = big page, 0 = small page
#define PAGE_PS (unsigned long)0x0080

// bit 6 Dirty: 1 = dirty, 0 = clean
#define PAGE_DIRTY (unsigned long)0x0040

// bit 5 Accessed: 1 = accessed, 0 = not accessed
#define PAGE_ACCESSED (unsigned long)0x0020

// bit 4 Page Level Cache Disable
#define PAGE_PCD (unsigned long)0x0010

// bit 3 Page Level Write Through
#define PAGE_PWT (unsigned long)0x0008

// bit 2 User/Supervisor: 1 = user & supervisor, 0 = supervisor
#define PAGE_US (unsigned long)0x0004

// bit 1 Read & Write: 1 = read & write, 0 = readonly
#define PAGE_RW (unsigned long)0x0002

// bit 0 Present: 1 = present, 0 = not present
#define PAGE_PRESENT (unsigned long)0x0001

#define PG_PTABLE_MAPED (1 << 0)
#define PG_KERNEL_INIT (1 << 1)
#define PG_REFERENCED (1 << 2)
#define PG_DIRTY (1 << 3)
#define PG_ACTIVE (1 << 4)
#define PG_UP_TO_DATE (1 << 5)
#define PG_DEVICE (1 << 6)
#define PG_KERNEL (1 << 7)
#define PG_K_SHARE_TO_U (1 << 8)
#define PG_SLAB (1 << 9)

#define ZONE_DMA (1 << 0)
#define ZONE_NORMAL (1 << 1)
#define ZONE_UNMAPED (1 << 2)

#define MAX_NR_ZONES 10

#define flush_tlb()               \
    do                            \
    {                             \
        unsigned long tmpreg;     \
        __asm__ __volatile__(     \
            "movq %%cr3, %0 \n\t" \
            "movq %0 ,%%cr3 \n\t" \
            : "=r"(tmpreg)        \
            :                     \
            : "memory");          \
    } while (0);

struct E820
{
    unsigned long address;
    unsigned long length;
    unsigned int type;
} __attribute__((packed));

struct GlobalMemoryDescriptor
{
    struct E820 e820[32];
    unsigned long e820_length;

    unsigned long *bits_map;
    unsigned long bits_size;
    unsigned long bits_length;

    struct Page *pages;
    unsigned long pages_size;
    unsigned long pages_length;

    struct Zone *zones;
    unsigned long zones_size;
    unsigned long zones_length;

    unsigned long start_code, end_code, end_data, end_brk;

    unsigned long end_of_struct;
};

struct Zone
{
    struct Page *pages;
    unsigned long pages_length;

    unsigned long zone_start_address;
    unsigned long zone_end_address;
    unsigned long zone_length;
    unsigned long attribute;

    struct GlobalMemoryDescriptor *gmd_struct;

    unsigned long pages_using_count;
    unsigned long pages_free_count;

    unsigned long pages_reference_count;
};

struct Page
{
    struct Zone *zone;
    unsigned long phys_address;
    unsigned long attribute;

    unsigned long reference_count;

    unsigned long age;
};

extern struct GlobalMemoryDescriptor memory_management_struct;

extern int ZONE_DMA_INDEX;
extern int ZONE_NORMAL_INDEX;
extern int ZONE_UNMAPED_INDEX;

extern unsigned long *global_cr3;

void init_memory();

unsigned long page_init(struct Page *page, unsigned long flags);

struct Page *alloc_pages(int zone_select, int number, unsigned long page_flags);

static inline unsigned long *get_gdt()
{
    unsigned long *tmp;
    __asm__ __volatile__(
        "movq %%cr3, %0 \n\t"
        : "=r"(tmp)
        :
        : "memory");
    return tmp;
}
