NASM = nasm

%.bin: %.asm
	$(NASM) -f bin $< -o $@ -l $(basename $<).lst

clean:
	rm -f *.bin *.lst

mbr: boot.bin
	dd if=boot.bin of=../bochs-vm/boot.img bs=512 count=1 conv=notrunc

all: loader.bin boot.bin
