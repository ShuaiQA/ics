#include <am.h>
#include <klib.h>
#include <nemu.h>
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
  // 写到寄存器中:最高位是mode=1,将sapt寄存器中左移12位才代表着页表目录的物理地址
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

  set_satp(kas.ptr);
  vme_enable = 1;

  return true;
}

// 设置用户虚拟地址空间(将操作系统的页目录进行复制),area(RANGE(0x40000000,
// 0x80000000)) pgsize == 4096
void protect(AddrSpace *as) {
  PTE *updir = (PTE *)(pgalloc_usr(PGSIZE));
  as->ptr = updir;
  as->area = USER_SPACE;
  as->pgsize = PGSIZE;
  // map kernel space
  memcpy(updir, kas.ptr, PGSIZE);
}

void unprotect(AddrSpace *as) {}

// 当前进程中的页目录地址保存到当前Context上下文中
void __am_get_cur_as(Context *c) {
  c->pdir = (vme_enable ? (void *)get_satp() : NULL);
}

// 将当前进程中的pdir指针保存到寄存器satp中
void __am_switch(Context *c) {
  if (vme_enable && c->pdir != NULL) {
    set_satp(c->pdir);
  }
}

// 将虚拟地址va映射到pa所在的物理地址
// 主要是进行初始化的过程,如果当前的页目录的页表项的有效位是0,就创建一个页表项设置为1然后修改该页表项的PPN和有效位
// 创建好页表之后就是设置页表的页表项和物理地址的对应,修改页表的页表项的PPN和有效位
void map(AddrSpace *as, void *va, void *pa, int prot) {
  PTE *pte = as->ptr; // 找的是页目录的地址
  // 当前的页表项的下一个页面是无效的则创建一个页面,然后设置相关的PPN
  while ((pte[(uintptr_t)va >> 22] & 0x1) == 0) { // 设置页目录
    void *next_add = pgalloc_usr(PGSIZE);
    pte[(uintptr_t)va >> 22] = (uintptr_t)next_add + 0x1;
  }
  PTE p = pte[(uintptr_t)va >> 22];
  PTE *next = (PTE *)(p & 0xfffff000);                   // 获取页表地址
  while ((next[(uintptr_t)va << 10 >> 22] & 0x1) == 0) { // 设置页表
    next[(uintptr_t)va << 10 >> 22] = ((uintptr_t)pa & 0xfffff000) + 0x1;
  }
  if (&kas != as) {
    printf("va is %x dir is %x table is %x pa is %x\n", va, pte, next, pa);
  }
}

// 设置了用户进程的mepc寄存器的值,为了上下文切换之后能够设置正确的pc值
// 将kstack的用户栈空间映射到[as.area.end - 32KB, as.area.end)这段虚拟地址空间.
// 设置进程的页目录的地址空间
// 返回Context的地址空间
Context *ucontext(AddrSpace *as, Area kstack, void *entry) {
  Context *ans = (Context *)kstack.end - 1; // 上下文的地址处
  // 设置mepc也就是恢复上下文之后的pc值(因为系统调用使用的是yield进行恢复)
  ans->mepc = (uintptr_t)entry;
  // 建立栈空间的虚拟映射
  void *va = as->area.end - 8 * PGSIZE;
  for (void *start = kstack.start; start < kstack.end; start += PGSIZE) {
    map(as, va, start, 0);
    va += PGSIZE;
  }
  ans->pdir = as->ptr;
  return ans;
}
