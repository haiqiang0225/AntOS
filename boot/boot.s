; --------------------------------------------------------------------------------------
;
;   boot.s -- 内核引导代码
;
; --------------------------------------------------------------------------------------

; Multiboot1 规范魔数
MBOOT_HEADER_MAGIC  equ     0x1BADB002

; 0 号位表示所有的引导模块将按页(4KB)边界对齐
MBOOT_PAGE_ALIGN    equ     1 << 0

; 1 号位通过 Multiboot 信息结构的 mem_* 域包括可用内存的信息
MBOOT_MEM_INFO      equ     1 << 1

; Multiboot1 规范检验和
MBOOT_HEADER_FLAGES equ     MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO

; 域‘checksum’是一个32位无符号值，当加上其它魔数域（即，‘magic’及‘flags’）时，
; 其和必须是无符号32位数0。
MBOOT_CHECKSUm      equ     -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGES)


; --------------------------------------------------------------------------------------
[BITS 32]   ; 以32位方式编译代码
section .init.text   ; 代码段开始

; 设置符合Multiboot 规范的标记

dd MBOOT_HEADER_MAGIC   ; GRUB魔数
dd MBOOT_HEADER_FLAGES  ; GRUB选项
dd MBOOT_CHECKSUm       ; GRUB检验和

[GLOBAL start]          ; 声明内核代码入口，此处提供该生命给链接器
[GLOBAL mboot_ptr_tmp]  ; 向外部声明 struct multiboot * 变量
[EXTERN kern_entry]     ; 声明内核C代码的入口函数

start:
    cli                         ; 关中断
    mov [mboot_ptr_tmp], ebx    ; 将 ebx 中存储的指针存入 glb_mboot_ptr 变量
    mov esp, STACK_TOP          ; 设置内核栈地址，按照 multiboot 规范，当需要使用堆栈时，OS 映象必须自己创建一个
    and esp, 0FFFFFFF0H         ; 栈地址按照 16 字节对齐
    mov ebp, 0                  ; 帧指针修改为 0

    call kern_entry             ; 调用内核入口函数

stop:
    hlt                         ; 休眠，直到下一个硬件中断被唤醒
    jmp stop                    ; 

; --------------------------------------------------------------------------------------

section .init.data		    ; 开启分页前临时的数据段
stack:    times 1024 db 0  	; 这里作为临时内核栈
STACK_TOP equ $-stack-1 	; 内核栈顶，$ 符指代是当前地址

mboot_ptr_tmp: dd 0		    ; 全局的 multiboot 结构体指针

; --------------------------------------------------------------------------------------

