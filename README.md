# x86_64-os

## Description

This is a simple x86_64 operating system written in C & ASM
Base on [一个64位操作系统的设计与实现](https://book.douban.com/subject/30222325/)

## How to build

1. Install nasm, make, binutils and gcc
2. Run following commands

```
cd ./bochs-vm
./gen-img
cd ../src/bootloader
make
make mbr
sudo make install
cd ../kernel
make
sudo make install
```

## How to run

Run following commands

```
cd ./bochs-vm
./run
```
