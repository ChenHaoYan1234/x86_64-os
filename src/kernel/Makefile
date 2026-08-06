CC = gcc
AS = as

head.o: head.S
	$(CC) -E $< > $(basename $<).s
	$(AS) --64 -o $@ $(basename $<).s
