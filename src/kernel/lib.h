#pragma once

#define NULL 0

static inline int strlen(char * string) {
    register int __res;
    __asm__ __volatile__ (
        "cld \n\t"
        "repne \n\t"
        "scasb \n\t"
        "notl %0 \n\t"
        "decl %0 \n\t"
        : "=c"(__res)
        : "D"(string), "a"(0), "0"(0xffffffff)
        :
    );
    return __res;
}
