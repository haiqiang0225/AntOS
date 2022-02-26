#include "vmm.h"
#include "debug.h"
#include "idt.h"
#include "pmm.h"
#include "string.h"

// 内核页目录，起始地址按照4KB对齐
pgd_t pgd_kern[PGD_SIZE] __attribute__((aligned(PAGE_SIZE)));

// 内核页表
static pte_t pte_kern[PTE_COUNT][PTE_SIZE] __attribute__((aligned(PAGE_SIZE)));

void init_vmm()
{
    /**
     * @brief
     * 这里要做的事情就是把虚拟地址3G以上的内核地址空间映射到物理内存，
     * 并且物理内存是从0开始映射的，也就是说这1G的内核空间是映射到物理
     * 内存0~1G这部分的
     */
    // 0xC0000000，内核虚拟内存的起始地址
    uint32_t kern_pte_first_idx = PGD_INDEX(PAGE_OFFSET);

    uint32_t i, j;
    for (i = kern_pte_first_idx, j = 0; i < PTE_COUNT + kern_pte_first_idx;
         i++, j++) {
        pgd_kern[i] = ((uint32_t)pte_kern[j] - PAGE_OFFSET) | PAGE_PRESENT | PAGE_WRITE;
    }

    uint32_t* pte = (uint32_t*)pte_kern;
    // 不映射第0页, 方便追踪NULL指针
    for (i = 1; i < PTE_COUNT * PTE_SIZE; i++) {
        pte[i] = (i << 12) | PAGE_PRESENT | PAGE_WRITE;
    }

    uint32_t pgd_kern_phy_addr = (uint32_t)pgd_kern - PAGE_OFFSET;

    // 注册缺页中断的处理函数 （14是缺页故障的中断号）
    register_interrupt_handler(14, &page_fault);

    // 将页目录的地址加载到cr3寄存器
    switch_pgd(pgd_kern_phy_addr);
}

void switch_pgd(uint32_t pd)
{
    asm volatile("mov %0, %%cr3"
                 :
                 : "r"(pd));
}

void map(pgd_t* pgd_now, uint32_t va, uint32_t pa, uint32_t flags)
{
    /**
     * @brief
     * 使用 flags 指出的页权限，把物理地址 pa 映射到虚拟地址 va
     * 1.先根据pgd_idx获取页表目录中的对应表项，这个表项的值就是下一级页表的位置
     * 2.按照1中找到的页表和对应的页表项索引pte_idx就可以获取到页表中的表项pte
     * 3.pte中存储着物理页号，加上offset即完成了虚拟地址到物理地址的转换
     */
    // 获取页表目录中pde的index，值为下一级页表
    uint32_t pgd_idx = PGD_INDEX(va);
    // 获取页表中
    uint32_t pte_idx = PTE_INDEX(va);

    // 从页目录中获取对应的页表
    pte_t* pt = (pte_t*)(pgd_now[pgd_idx] & PAGE_MASK);
    if (!pt) { // 如果这个页表还未分配
        // 分配一个物理内存页
        pt = (pte_t*)pmm_alloc_page();

        // 将这个物理页的基址和对应的标志位写入到页目录中
        pgd_now[pgd_idx] = (uint32_t)pt | PAGE_PRESENT | PAGE_WRITE;

        // 转换到内核线性地址并清 0
        pt = (pte_t*)((uint32_t)pt + PAGE_OFFSET);
        bzero(pt, PAGE_SIZE);
    } else {
        // 转换到内核线性地址
        pt = (pte_t*)((uint32_t)pt + PAGE_OFFSET);
    }

    pt[pte_idx] = (pa & PAGE_MASK) | flags;

    // 通知 CPU 更新页表缓存
    asm volatile("invlpg (%0)"
                 :
                 : "a"(va));
}

void unmap(pgd_t* pgd_now, uint32_t va)
{
    uint32_t pgd_idx = PGD_INDEX(va);
    uint32_t pte_idx = PTE_INDEX(va);

    pte_t* pte = (pte_t*)(pgd_now[pgd_idx] & PAGE_MASK);

    if (!pte) {
        return;
    }

    // 转换到内核线性地址
    pte = (pte_t*)((uint32_t)pte + PAGE_OFFSET);

    pte[pte_idx] = 0;

    // 通知 CPU 更新页表缓存
    asm volatile("invlpg (%0)"
                 :
                 : "a"(va));
}

uint32_t get_mapping(pgd_t* pgd_now, uint32_t va, uint32_t* pa)
{
    uint32_t pgd_idx = PGD_INDEX(va);
    uint32_t pte_idx = PTE_INDEX(va);

    pte_t* pte = (pte_t*)(pgd_now[pgd_idx] & PAGE_MASK);
    if (!pte) {
        return 0;
    }

    // 转换到内核线性地址
    pte = (pte_t*)((uint32_t)pte + PAGE_OFFSET);

    // 如果地址有效而且指针不为NULL，则返回地址
    if (pte[pte_idx] != 0 && pa) {
        *pa = pte[pte_idx] & PAGE_MASK;
        return 1;
    }

    return 0;
}
