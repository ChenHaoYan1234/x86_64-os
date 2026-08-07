#pragma once

#define _set_gate(gate_selector_addr, attr, ist, code_addr)                        \
    do                                                                             \
    {                                                                              \
        unsigned long __d0, __d1;                                                  \
        __asm__ __volatile__("movw %%dx, %%ax \n\t"                                \
                             "andq $0x7, %%rcx \n\t"                               \
                             "addq %4, %%rcx \n\t"                                 \
                             "shlq $32, %%rcx \n\t"                                \
                             "addq %%rcx, %%rax \n\t"                              \
                             "xorq %%rcx, %%rcx \n\t"                              \
                             "movl %%edx, %%ecx \n\t"                              \
                             "shrq $16, %%rcx \n\t"                                \
                             "shlq $48, %%rcx \n\t"                                \
                             "addq %%rcx, %%rax \n\t"                              \
                             "movq %%rax, %0 \n\t"                                 \
                             "shrq $32, %%rdx \n\t"                                \
                             "movq %%rdx, %1 \n\t"                                 \
                             : "=m"(*((unsigned long *)(gate_selector_addr))),     \
                               "=m"(*(1 + (unsigned long *)(gate_selector_addr))), \
                               "=&a"(__d0), "=&d"(__d1)                            \
                             : "i"(attr << 8),                                     \
                               "3"((unsigned long *)(code_addr)),                  \
                               "2"(0x8 << 16), "c"(ist)                            \
                             : "memory");                                          \
    } while (0)

#define load_TR(n)                         \
    do                                     \
    {                                      \
        __asm__ __volatile__("ltr %%ax"    \
                             :             \
                             : "a"(n << 3) \
                             : "memory");  \
    } while (0)

struct GateStruct
{
    unsigned char x[16];
};

extern struct GateStruct idt_table[];
extern unsigned int tss64_table[26];

static inline void set_intr_gate(unsigned int n, unsigned char ist, void *addr)
{
    _set_gate(idt_table + n, 0x8e, ist, addr); // P,DPL=0, TYPE=E
}

static inline void set_trap_gate(unsigned int n, unsigned char ist, void *addr)
{
    _set_gate(idt_table + n, 0x8f, ist, addr); // P,DPL=0, TYPE=F
}

static inline void set_system_gate(unsigned int n, unsigned char ist, void *addr)
{
    _set_gate(idt_table + n, 0xef, ist, addr); // P,DPL=3, TYPE=F
}

void set_tss64(unsigned long rsp0, unsigned long rsp1, unsigned long rsp2,
               unsigned long ist1, unsigned long ist2, unsigned long ist3,
               unsigned long ist4, unsigned long ist5, unsigned long ist6,
               unsigned long ist7)
{
    *(unsigned long *)(tss64_table + 1) = rsp0;
    *(unsigned long *)(tss64_table + 3) = rsp1;
    *(unsigned long *)(tss64_table + 5) = rsp2;

    *(unsigned long *)(tss64_table + 9) = ist1;
    *(unsigned long *)(tss64_table + 11) = ist2;
    *(unsigned long *)(tss64_table + 13) = ist3;
    *(unsigned long *)(tss64_table + 15) = ist4;
    *(unsigned long *)(tss64_table + 17) = ist5;
    *(unsigned long *)(tss64_table + 19) = ist6;
    *(unsigned long *)(tss64_table + 21) = ist7;
}
