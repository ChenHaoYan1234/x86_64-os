CC = gcc
AS = as
LD = ld
OBJCOPY = objcopy

CCFLAGS = -fno-stack-protector -mcmodel=large -fno-builtin -m64 -Wall
LDFLAGS = -b elf64-x86-64  -z muldefs -T kernel.lds

OBJS = head.o $(patsubst %.c, %.o, $(wildcard *.c))

all: kernel.bin

head.o: head.S
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
