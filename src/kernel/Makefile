CC = gcc
AS = as
LD = ld
OBJCOPY = objcopy

INCLUDE_FLAGS = -I./includes

CCFLAGS = -fno-stack-protector -mcmodel=large -fno-builtin -m64 -Wall -Werror
LDFLAGS = -b elf64-x86-64  -z muldefs -T kernel.lds

OBJS_C = $(patsubst %.c, %.o, $(wildcard *.c))
OBJS_ASM = asm/head.o $(filter-out asm/head.o, $(patsubst asm/%.S, asm/%.o, $(wildcard asm/*.S)))

IMG = ../../bochs-vm/boot.img

all: kernel.bin

%.o: %.S
	$(CC) $(INCLUDE_FLAGS) -E $< > $(basename $<).s
	$(AS) --64 -o $@ $(basename $<).s

%.o: %.c
	$(CC) $(INCLUDE_FLAGS) $(CCFLAGS) -c $< -o $@

system: $(OBJS_ASM) $(OBJS_C) kernel.lds
	$(LD) $(LDFLAGS) -o system $(OBJS_ASM) $(OBJS_C)

kernel.bin: system
	$(OBJCOPY) -I elf64-x86-64 -S -R ".eh_frame" -R ".comment" -O binary system $@

clean:
	rm -f *.o kernel.bin system
	rm -f asm/*.o asm/*.s

install: all
	mount $(IMG) /mnt -t vfat -o loop
	cp kernel.bin /nt
	sync
	umount /mnt

rebuild: clean all
