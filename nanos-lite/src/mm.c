#include "klib-macros.h"
#include <memory.h>
#include <string.h>

static void *pf = NULL;

// 分配多少的页面数目
void *new_page(size_t nr_page) {
  void *ret = pf;
  pf += PGSIZE * nr_page;
  return ret;
}

#ifdef HAS_VME
// 分配多少字节数目
static void *pg_alloc(int n) {
  void *mem = new_page(ROUNDUP(n, PGSIZE)/PGSIZE);
  return memset(mem, 0, PGSIZE);
}
#endif

void free_page(void *p) { panic("not implement yet"); }

/* The brk() system call handler. */
int mm_brk(uintptr_t brk) { return 0; }

void init_mm() {
  // 获取页面的初始化的地址
  pf = (void *)ROUNDUP(heap.start, PGSIZE);
  Log("free physical pages starting from %p", pf);

#ifdef HAS_VME
  vme_init(pg_alloc, free_page);
#endif
}
