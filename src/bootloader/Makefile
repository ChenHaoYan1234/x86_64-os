NASM = nasm

IMG = ../../bochs-vm/bochs.img

all: loader.bin boot.bin

%.bin: %.asm
	$(NASM) -f bin $< -o $@ -l $(basename $<).lst

clean:
	rm -f *.bin *.lst

rebuild: clean all

mbr: boot.bin
	dd if=boot.bin of=$(IMG) bs=512 count=1 conv=notrunc

install: loader.bin
	mount $(IMG) /mnt -t vfat -o loop
	cp loader.bin /nt
	sync
	umount /mnt
