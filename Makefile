#!Makefile

C_SOURCES = $(shell find . -name "*.c")
C_OBJECTS = $(patsubst %.c, %.o, $(C_SOURCES))
S_SOURCES = $(shell find . -name "*.s")
S_OBJECTS = $(patsubst %.c, %.o, $(S_SOURCES))


CC = gcc
LD = ld
ASM = nasm

# -c 不进行链接 -Wall 允许所有用户需要考虑的关于构建的警告信息, 产生尽可能多的警告信息
# -m32 生成32位汇编 -ggdb 生成用于gdb的调试信息 -gstabs+ 生成stabs格式的调试信息
# -nostdinc 不要在标准系统目录查找头文件，只在-I选项后指定的路径查找
# -fno-pic
# -fno-builtin 不使用不以”__builtin_“为起始的内建函数
# -I 指定头文件路径
C_FLAGS = -c -Wall -m32 -ggdb -gstabs+ -nostdinc -fno-pic -fno-builtin -fno-stack-protector -I include
# -T 使用指定的链接脚本 -m elf_i386 生成i386平台下的elf格式可执行文件 -nostdlib 不链接C语言的标准库
LD_FLAGS = -T scripts/kernel.ld -m elf_i386 -nostdlib
# -f 指定输出文件的格式 -g 允许生成调试信息 -F 选择调试信息的格式
ASM_FLAGS = -f elf -g -F stabs

all : $(S_OBJECTS) $(C_OBJECTS)


# 这个标签等价于 %.o:%.c
.c.o:
	@echo compile source code file $< ...
	$(CC) $(C_FLAGS) $< -o $@

.s.o:
	@echo compile assembly file $<
	$(ASM) $(ASM_FLAGS) $<

link:
	@echo link kernel file ...
	$(LD) $(LD_FLAGS) $(S_OBJECTS) $(C_OBJECTS) -o K_kernel

.PHONY:clean
clean:
	$(RM) $(S_OBJECTS) $(C_OBJECTS) K_kernel
	
.PHONY:update_image
update_image:
	sudo mount floppy,img /mnt/kernel
	sudo cp hx_kernel /mnt/kernel/K_kernel
	sleep 1
	sudo umount /mnt/kernel

.PHONY:mount_image
mount_image:
	sudo mount floppy.img /mnt/kernel

.PHONY:unmont_image
unmount_image:
	sudo umount /mnt/kernel

.PHONY:qemu
qemu:
	qemu -fda floppy.img -boot a

.PHONY:bochs
bochs:
	bochs -f tools/bochsrc.txt

.PHONY:debug
debug:
	qemu -S -s -fda floppy.img -boot a &
	sleep 1
	cgdb -x tools/gdbinit

