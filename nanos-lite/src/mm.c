#include <am.h>
#include <debug.h>
#include <memory.h>

static void *pf = NULL;

// new_page()函数来获得用户栈的内存空间
// 通过一个pf指针来管理堆区, 用于分配一段大小为nr_page * 4KB的连续内存区域,
// 并返回这段区域的首地址(类似于malloc)
void *new_page(size_t nr_page) {
  void *older = pf;
  pf += nr_page * PGSIZE;
  return older;
}

#ifdef HAS_VME
static void *pg_alloc(int n) { return new_page(n / PGSIZE); }
#endif

void free_page(void *p) { panic("not implement yet"); }

/* The brk() system call handler. */
int mm_brk(uintptr_t brk) { return 0; }

void init_mm() {
  pf = (void *)ROUNDUP(heap.start, PGSIZE);
  Log("heap.start address is %p free physical pages starting from %p",
      heap.start, pf);

#ifdef HAS_VME
  vme_init(pg_alloc, free_page);
#endif
}
