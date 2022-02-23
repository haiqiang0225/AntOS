#include "console.h"
#include "debug.h"
#include "gdt.h"

int kern_entry()
{
    init_debug();
    init_gdt();
    
    console_clear();

    printk_color(rc_black, rc_green, "Hello, OS kernel!\n");

    panic("test");

    return 0;
}