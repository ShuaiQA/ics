#include <am.h>
#include <klib.h>
#include <nemu.h>

static AddrSpace kas = {};
static void *(*pgalloc_usr)(int) = NULL;
static void (*pgfree_usr)(void *) = NULL;
static int vme_enable = 0;

static Area segments[] = { // Kernel memory mappings
    NEMU_PADDR_SPACE};

#define USER_SPACE RANGE(0x40000000, 0x80000000)

static inline void set_satp(void *pdir) {
  uintptr_t mode = 1ul << (__riscv_xlen - 1);
  // 写入什么到寄存器中:最高位是mode=1,将sapt寄存器中左移12位才代表着页表目录的物理地址
  asm volatile("csrw satp, %0" : : "r"(mode | ((uintptr_t)pdir >> 12)));
}

static inline uintptr_t get_satp() {
  uintptr_t satp;
  asm volatile("csrr %0, satp" : "=r"(satp));
  return satp << 12;
}

bool vme_init(void *(*pgalloc_f)(int), void (*pgfree_f)(void *)) {
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  kas.ptr = pgalloc_f(PGSIZE);

  int i;
  for (i = 0; i < LENGTH(segments); i++) {
    void *va = segments[i].start;
    for (; va < segments[i].end; va += PGSIZE) {
      map(&kas, va, va, 0);
    }
  }

  printf("over init\n");
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

// 将虚拟地址va映射到pa所在的物理地址
// 主要是进行初始化的过程,如果当前的页目录的页表项的有效位是0,就创建一个页表项设置为1然后修改该页表项的PPN和有效位
// 创建好页表之后就是设置页表的页表项和物理地址的对应,修改页表的页表项的PPN和有效位
void map(AddrSpace *as, void *va, void *pa, int prot) {
  PTE *pte = as->ptr;
  // 当前的页表项的下一个页面是无效的则创建一个页面,然后设置相关的PPN
  while ((pte[(int)va >> 22] & 0x1) != 0) { // 设置页目录
    void *next_add = pgalloc_usr(PGSIZE);
    pte[(int)va >> 22] = ((int)next_add & 0xfffff000) + 0x1;
  }
  PTE p = pte[(int)va >> 22];
  PTE *next = (PTE *)(p & 0xfffff000);             // 获取页表
  while ((next[(int)va << 10 >> 22] & 0x1) != 0) { // 设置页表
    int pyadd = (int)pa >> 12;
    next[(int)va << 10 >> 22] = (pyadd & 0xfffff000) + 0x1;
  }
}

// 创建用户进程的上下文,注意当前是内核的PCB进程控制块的管理,目前和用户进程的设置没有关系
Context *ucontext(AddrSpace *as, Area kstack, void *entry) {
  // 获取最后一个Context空间
  Context *ans = (Context *)kstack.end - 1;
  // 设置mepc也就是恢复上下文之后的pc值,以及sp寄存器的值(栈指针寄存器,用来分配局部变量)
  ans->mepc = (uintptr_t)entry;
  return ans;
}
