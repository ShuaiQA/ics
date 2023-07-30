#include <am.h>
#include <klib.h>
#include <nemu.h>
#include <stddef.h>
#include <stdint.h>

static AddrSpace kas = {};
static void *(*pgalloc_usr)(int) = NULL;
static void (*pgfree_usr)(void *) = NULL;
static int vme_enable = 0;

static Area segments[] = { // Kernel memory mappings
    NEMU_PADDR_SPACE};

#define USER_SPACE RANGE(0x40000000, 0x80000000)

static inline void set_satp(void *pdir) {
  uintptr_t mode = 1ul << (__riscv_xlen - 1);
  asm volatile("csrw satp, %0" : : "r"(mode | ((uintptr_t)pdir >> 12)));
}

static inline uintptr_t get_satp() {
  uintptr_t satp;
  asm volatile("csrr %0, satp" : "=r"(satp));
  return satp << 12;
}

// 传入的参数是分配和回收页面函数
bool vme_init(void *(*pgalloc_f)(int), void (*pgfree_f)(void *)) {
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  // 获取一个页面,放到ptr中
  kas.ptr = pgalloc_f(PGSIZE);
  int i;
  // 创建虚拟地址恒等映射
  for (i = 0; i < LENGTH(segments); i++) {
    void *va = segments[i].start;
    for (; va < segments[i].end; va += PGSIZE) {
      map(&kas, va, va, 0);
    }
  }
  // 设置satp寄存器的值,开启虚拟映射
  printf("begin kas\n");
  set_satp(kas.ptr);
  vme_enable = 1;

  return true;
}

void protect(AddrSpace *as) {
  PTE *updir = (PTE *)(pgalloc_usr(PGSIZE));
  as->ptr = updir;
  as->area = USER_SPACE;
  as->pgsize = PGSIZE;
  // map kernel space
  memcpy(updir, kas.ptr, PGSIZE);
}

void unprotect(AddrSpace *as) {}

void __am_get_cur_as(Context *c) {
  c->pdir = (vme_enable ? (void *)get_satp() : NULL);
}

void __am_switch(Context *c) {
  if (vme_enable && c->pdir != NULL) {
    set_satp(c->pdir);
  }
}

#define PXMASK 0x1FF // 9 bits
#define PGSHIFT 12   // bits of offset within a page
#define PXSHIFT(level) (PGSHIFT + (9 * (level)))
#define PX(level, va) ((((uintptr_t)(va)) >> PXSHIFT(level)) & PXMASK)
#define PTE2PA(pte) (((pte) >> 10) << 12) // 根据页表项获取物理地址
#define PA2PTE(pa) ((((uintptr_t)pa) >> 12) << 10)
typedef uintptr_t *pagetable_t; // 512 PTEs

PTE *walk(pagetable_t pagetable, void *va, int alloc) {
  for (int level = 2; level > 0; level--) {
    PTE *pte = &pagetable[PX(level, va)];
    if (*pte & PTE_V) {
      pagetable = (pagetable_t)PTE2PA(*pte);
    } else {
      if (!alloc || (pagetable = (pagetable_t)pgalloc_usr(PGSIZE)) == 0)
        return 0;
      *pte = PA2PTE(pagetable) | PTE_V;
    }
  }
  return &pagetable[PX(0, va)];
}

// 获取as的页目录,然后添加根据va虚拟地址和pa物理地址建立虚拟映射关系
void map(AddrSpace *as, void *va, void *pa, int prot) {
  PTE *pte;
  if ((pte = walk(as->ptr, va, 1)) == 0)
    panic("PTE error");
  if (*pte & PTE_V)
    panic("mappages: remap");
  *pte = PA2PTE(pa) | prot | PTE_V;
}

Context *ucontext(AddrSpace *as, Area kstack, void *entry) {
  Context *cte = (Context *)kstack.end - 1; // 上下文的地址处
  cte->mepc = (uintptr_t)entry;
  return cte;
}
