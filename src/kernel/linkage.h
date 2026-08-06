#pragma once

#define SYMBOL_NAME(x) x
#define SYMBOL_NAME_STR(x) #x
#define SYMBOL_NAME_LABEL(x) x##:

#define ENTRY(name)            \
    .global SYMBOL_NAME(name); \
    SYMBOL_NAME_LABEL(name)
