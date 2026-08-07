#include "lib.h"
#include "memory.h"
#include "printk.h"

int ZONE_DMA_INDEX = 0;
int ZONE_NORMAL_INDEX = 0;
int ZONE_UNMAPED_INDEX = 0;

unsigned long *global_cr3 = NULL;

void init_memory()
{
    int i, j;
    unsigned long total_mem = 0;
    struct E820 *p = NULL;

    color_printk(BLUE, BLACK, "Display Physical Address Map (1: RAM, 2: ROM or Reserved, 3: ACPI Reclaim Memory, 4: ACPI NVS Memory, Others: Undefined)\n");

    p = (struct E820 *)0xffff800000007e00;

    for (i = 0; i < 32; i++)
    {
        color_printk(ORANGE, BLACK, "Address: %#018lx\tLength: %#018lx\tType: %#010x\n",
                     p->address, p->length, p->type);

        if (p->type == 1)
            total_mem += p->length;

        memory_management_struct.e820[i].address += p->address;
        memory_management_struct.e820[i].length += p->length;
        memory_management_struct.e820[i].type = p->type;
        memory_management_struct.e820_length = i;

        p++;
        if (p->type > 4 || p->length == 0 || p->type < 1)
            break;
    }

    color_printk(GREEN, BLACK, "Total RAM: %#018lx\n", total_mem);

    total_mem = 0;
    for (i = 0; i <= memory_management_struct.e820_length; i++)
    {
        unsigned long start, end;

        if (memory_management_struct.e820[i].type != 1)
            continue;

        start = PAGE_2M_ALIGN(memory_management_struct.e820[i].address);
        end = ((memory_management_struct.e820[i].address + memory_management_struct.e820[i].length) >> PAGE_2M_SHIFT) << PAGE_2M_SHIFT;

        if (end <= start)
            continue;

        total_mem += (end - start) >> PAGE_2M_SHIFT;
    }

    color_printk(ORANGE, BLACK, "Total 2M PAGEs: %#010x=%010d\n", total_mem, total_mem);

    total_mem = memory_management_struct.e820[memory_management_struct.e820_length].address + memory_management_struct.e820[memory_management_struct.e820_length].length;

    memory_management_struct.bits_map = (unsigned long *)((memory_management_struct.end_brk + PAGE_4K_SIZE - 1) & PAGE_4K_MASK);
    memory_management_struct.bits_size = total_mem >> PAGE_2M_SHIFT;
    memory_management_struct.bits_length = (((unsigned long)(total_mem >> PAGE_2M_SHIFT) + sizeof(long) * 8 - 1) / 8) & (~(sizeof(long) - 1));

    memset(memory_management_struct.bits_map, 0xff, memory_management_struct.bits_length);

    memory_management_struct.pages = (struct Page *)(((unsigned long)memory_management_struct.bits_map + memory_management_struct.bits_length + PAGE_4K_SIZE - 1) & PAGE_4K_MASK);
    memory_management_struct.pages_size = total_mem >> PAGE_2M_SHIFT;
    memory_management_struct.pages_length = ((total_mem >> PAGE_2M_SHIFT) * sizeof(struct Page) + sizeof(long) - 1) & (~(sizeof(long) - 1));

    memset(memory_management_struct.pages, 0x00, memory_management_struct.pages_length);

    memory_management_struct.zones = (struct Zone *)(((unsigned long)memory_management_struct.pages + memory_management_struct.pages_length + PAGE_4K_SIZE - 1) & PAGE_4K_MASK);
    memory_management_struct.zones_size = 0;
    memory_management_struct.zones_length = (5 * sizeof(struct Zone) + sizeof(long) - 1) & (~(sizeof(long) - 1));

    memset(memory_management_struct.zones, 0x00, memory_management_struct.zones_length);

    for (i = 0; i <= memory_management_struct.e820_length; i++)
    {
        unsigned long start, end;
        struct Zone *z;
        struct Page *p;

        if (memory_management_struct.e820[i].type != 1)
            continue;

        start = PAGE_2M_ALIGN(memory_management_struct.e820[i].address);
        end = ((memory_management_struct.e820[i].address + memory_management_struct.e820[i].length) >> PAGE_2M_SHIFT) << PAGE_2M_SHIFT;

        if (end <= start)
            continue;

        z = memory_management_struct.zones + memory_management_struct.zones_size;
        memory_management_struct.zones_size++;

        z->zone_start_address = start;
        z->zone_end_address = end;
        z->zone_length = end - start;

        z->pages_using_count = 0;
        z->pages_free_count = (end - start) >> PAGE_2M_SHIFT;

        z->pages_reference_count = 0;

        z->attribute = 0;
        z->gmd_struct = &memory_management_struct;

        z->pages_length = (end - start) >> PAGE_2M_SHIFT;
        z->pages = (struct Page *)(memory_management_struct.pages + (start >> PAGE_2M_SHIFT));

        p = z->pages;
        for (j = 0; j < z->pages_length; j++, p++)
        {
            p->zone = z;
            p->phys_address = start + PAGE_2M_SIZE * j;
            p->attribute = 0;

            p->reference_count = 0;
            p->age = 0;

            *(memory_management_struct.bits_map + ((p->phys_address >> PAGE_2M_SHIFT) >> 6)) ^= 1UL << (p->phys_address >> PAGE_2M_SHIFT) % 64;
        }
    }

    // init address - to page struct 0 because the memory_management_struct.e820[0].type != 1
    memory_management_struct.pages->zone = memory_management_struct.zones;
    memory_management_struct.pages->phys_address = 0;
    memory_management_struct.pages->attribute = 0;
    memory_management_struct.pages->reference_count = 0;
    memory_management_struct.pages->age = 0;

    memory_management_struct.zones_length = (memory_management_struct.zones_size * sizeof(struct Zone) + sizeof(long) - 1) & (!(sizeof(long) - 1));

    color_printk(ORANGE, BLACK, "bits_map: %018lx, bits_size: %018lx, bits_length: %018lx\n", memory_management_struct.bits_map, memory_management_struct.bits_size, memory_management_struct.bits_length);
    color_printk(ORANGE, BLACK, "pages: %018lx, pages_size: %018lx, pages_length: %018lx\n", memory_management_struct.pages, memory_management_struct.pages_size, memory_management_struct.pages_length);
    color_printk(ORANGE, BLACK, "zones: %018lx, zones_size: %018lx, zones_length: %018lx\n", memory_management_struct.zones, memory_management_struct.zones_size, memory_management_struct.zones_length);

    ZONE_DMA_INDEX = 0;    // need rewrite in the future
    ZONE_NORMAL_INDEX = 0; // need rewrite in the future

    for (i = 0; i < memory_management_struct.zones_size; i++) // need rewrite in the future
    {
        struct Zone *z = memory_management_struct.zones + i;

        color_printk(ORANGE, BLACK, "zone_start_address: %#018lx, zone_end_address: %#018lx, zone_length: %#018lx, pages_group: %#018lx, pages_length: %#018lx\n",
                     z->zone_start_address, z->zone_end_address, z->zone_length, z->pages, z->pages_length);

        if (z->zone_start_address == 0x100000000)
            ZONE_UNMAPED_INDEX = i;
    }

    // need a blank to separate memory_management_struct
    memory_management_struct.end_of_struct = (unsigned long)((unsigned long)memory_management_struct.zones + memory_management_struct.zones_length + sizeof(long) * 32) & (~(sizeof(long) - 1));

    color_printk(ORANGE, BLACK, "start_code: %#018lx, end_code: %#018lx, end_data: %#018lx, end_brk: %#018lx, end_of struct: %#018lx\n",
                 memory_management_struct.start_code, memory_management_struct.end_code, memory_management_struct.end_data, memory_management_struct.end_brk, memory_management_struct.end_of_struct);

    i = VIRT_TO_PHYS(memory_management_struct.end_of_struct) >> PAGE_2M_SHIFT;

    for (j = 0; j <= i; j++)
    {
        page_init(memory_management_struct.pages + j, PG_PTABLE_MAPED | PG_KERNEL_INIT | PG_ACTIVE | PG_KERNEL);
    }

    global_cr3 = get_gdt();

    color_printk(INDIGO, BLACK, "global_cr3: %#018lx\n", global_cr3);
    color_printk(INDIGO, BLACK, "*global_cr3: %#018lx\n", *PHYS_TO_VIRT(global_cr3) & (~0xff));
    color_printk(INDIGO, BLACK, "**global_cr3: %#018lx\n", *PHYS_TO_VIRT(*PHYS_TO_VIRT(global_cr3) & (~0xff)) & (~0xff));

    for (i = 0; i < 10; i++)
        *(PHYS_TO_VIRT(global_cr3) + i) = 0UL;

    flush_tlb();
}

unsigned long page_init(struct Page *page, unsigned long flags)
{
    if (!page->attribute)
    {
        *(memory_management_struct.bits_map + ((page->phys_address >> PAGE_2M_SHIFT) >> 6)) |= 1UL << (page->phys_address >> PAGE_2M_SHIFT) % 64;
        page->attribute = flags;
        page->reference_count++;
        page->zone->pages_using_count++;
        page->zone->pages_free_count--;
        page->zone->pages_reference_count++;
    }
    else if ((page->attribute & PG_REFERENCED) || (page->attribute & PG_K_SHARE_TO_U) || (flags & PG_REFERENCED) || (flags & PG_K_SHARE_TO_U))
    {
        page->attribute |= flags;
        page->reference_count++;
        page->zone->pages_reference_count++;
    }
    else
    {
        *(memory_management_struct.bits_map + ((page->phys_address >> PAGE_2M_SHIFT) >> 6)) |= 1UL << (page->phys_address >> PAGE_2M_SHIFT) % 64;
        page->attribute |= flags;
    }
    return 0;
}

struct Page *alloc_pages(int zone_select, int number, unsigned long page_flags)
{
    int i;
    unsigned long page = 0;
    int zone_start = 0;
    int zone_end = 0;

    switch (zone_select)
    {
    case ZONE_DMA:
        zone_start = 0;
        zone_end = ZONE_DMA_INDEX;
        break;

    case ZONE_NORMAL:
        zone_start = ZONE_DMA_INDEX;
        zone_end = ZONE_NORMAL_INDEX;
        break;

    case ZONE_UNMAPED:
        zone_start = ZONE_NORMAL_INDEX;
        zone_end = memory_management_struct.zones_size - 1;
        break;

    default:
        color_printk(RED, BLACK, "alloc_pages error zone_select index\n");
        return NULL;
        break;
    }

    for (i = zone_start; i <= zone_end; i++)
    {
        struct Zone *z;
        unsigned long j;
        unsigned long start, end;
        // unsigned long length;
        unsigned long tmp;

        if ((memory_management_struct.zones + i)->pages_free_count < number)
            continue;

        z = memory_management_struct.zones + i;
        start = z->zone_start_address >> PAGE_2M_SHIFT;
        end = z->zone_end_address >> PAGE_2M_SHIFT;
        // length = z->zone_length >> PAGE_2M_SHIFT;

        tmp = 64 - start % 64;

        for (j = start; j <= end; j += j % 64 ? tmp : 64)
        {
            unsigned long *p = memory_management_struct.bits_map + (j >> 6);
            unsigned long shift = j % 64;
            unsigned long k;
            for (k = shift; k < 64 - shift; k++)
            {
                if (!(((*p >> k) | (*(p + 1) << (64 - k))) & (number == 64 ? 0xffffffffffffffffUL : ((1UL << number) - 1))))
                {
                    unsigned long l;
                    page = j + k - 1;
                    for (l = 0; l < number; l++)
                    {
                        struct Page *x = memory_management_struct.pages + page + l;
                        page_init(x, page_flags);
                    }
                    goto find_free_pages;
                }
            }
        }
    }

    return NULL;

find_free_pages:
    return (struct Page *)(memory_management_struct.pages + page);
}
