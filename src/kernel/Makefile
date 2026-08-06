CC = gcc
AS = as
LD = ld
OBJCOPY = objcopy

CCFLAGS = -fno-stack-protector -mcmodel=large -fno-builtin -m64 -Wall -Werror
LDFLAGS = -b elf64-x86-64  -z muldefs -T kernel.lds

OBJS_C = $(patsubst %.c, %.o, $(wildcard *.c))
OBJS_ASM = head.o $(filter-out head.o, $(patsubst %.S, %.o, $(wildcard *.S)))
OBJS = $(OBJS_ASM) $(OBJS_C)

all: kernel.bin

%.o: %.S
	$(CC) -E $< > $(basename $<).s
	$(AS) --64 -o $@ $(basename $<).s

%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@

system: $(OBJS) kernel.lds
	$(LD) $(LDFLAGS) -o system $(OBJS)

kernel.bin: system
	$(OBJCOPY) -I elf64-x86-64 -S -R ".eh_frame" -R ".comment" -O binary system $@

clean:
	rm -f *.o *.s kernel.bin system

rebuild: clean all
