# nasm -I include/ -o mbr.bin mbr.S 
# nasm -I include/ -o loader.bin  loader.S
# dd if=mbr.bin of=hd60M.img bs=512 count=1 conv=notrunc
# dd if=loader.bin of=hd60M.img bs=512 count=4 seek=2 conv=notrunc
gcc -m32 -I lib/kernel/ -m32 -I lib/ -m32 -I kernel/ -c -fno-builtin -fno-stack-protector -o build/main.o kernel/main.c
gcc -m32 -I lib/kernel/ -m32 -I lib/ -m32 -I kernel/ -c -fno-builtin -fno-stack-protector -o build/interrupt.o kernel/interrupt.c
gcc -m32 -I lib/kernel/ -m32 -I lib/ -m32 -I kernel/ -c -fno-builtin -fno-stack-protector -o build/init.o kernel/init.c
gcc -m32 -I lib/kernel/ -c -fno-builtin -fno-stack-protector -o build/timer.o device/timer.c
nasm -f elf -o build/print.o lib/kernel/print.S
nasm -f elf -o build/kernel.o kernel/kernel.S
ld -m elf_i386 -Ttext 0xc0001500 -e main -o build/kernel.bin build/main.o build/init.o build/interrupt.o build/print.o build/kernel.o
dd if=build/kernel.bin of=hd60M.img bs=512 count=200 seek=9 conv=notrunc
bin/bochs -f bochsrc.disk
