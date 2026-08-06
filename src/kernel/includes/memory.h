#pragma once

struct Memory_E820_Format {
    unsigned int address1;
    unsigned int address2;
    unsigned int length1;
    unsigned int length2;
    unsigned int type;
};

void init_memory();
