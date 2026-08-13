#pragma once

#include "linkage.h"

void init_interrupt();

void do_IRQ(unsigned long regs, unsigned long nr);
