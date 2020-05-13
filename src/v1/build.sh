#!/bin/bash
#assemble bootloader (boot.s) file
echo ">>> Assembling bootloader..."
as --32 boot.s -o boot.o
echo "Success."
#compile kernel (kernel.c) and libraries (lib.h, keyboard.h, types.h, lib.c)
echo ">>> Compiling kernel..."
gcc -m32 -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O1 -Wall -Wextra
gcc -m32 -c lib.c -o lib.o -std=gnu99 -ffreestanding -O1 -Wall -Wextra
gcc -m32 -c keychar.c -o keychar.o -std=gnu99 -ffreestanding -O1 -Wall -Wextra
echo "Success."
#link the kernel with kernel.o and boot.o files
echo ">>> Linking kernel..."
ld -m elf_i386 -T linker.ld kernel.o lib.o keychar.o boot.o -o cbc.bin -nostdlib
echo "Success"
#check whether cbc.bin is a 80x86 (x86) multiboot file or not.
grub-file --is-x86-multiboot cbc.bin
#add the grub config file and cbc.bin file to .iso image 
echo ">>> Making ISO..."
rm cbc.iso
mkdir -p isodir/boot/grub
cp cbc.bin isodir/boot/cbc.bin
cp grub.cfg isodir/boot/grub/grub.cfg
grub-mkrescue -o cbc.iso 
grub-mkrescue -o cbc.iso isodir
echo ""Success. Cadex Built Sucessfully.
