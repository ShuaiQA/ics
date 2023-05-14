#include <memory.h>

static void *pf = NULL;

// new_page()函数来获得用户栈的内存空间
// 通过一个pf指针来管理堆区, 用于分配一段大小为nr_page * 4KB的连续内存区域,
// 并返回这段区域的首地址
void *new_page(size_t nr_page) { return NULL; }

#ifdef HAS_VME
static void *pg_alloc(int n) { return NULL; }
#endif

void free_page(void *p) { panic("not implement yet"); }

/* The brk() system call handler. */
int mm_brk(uintptr_t brk) { return 0; }

void init_mm() {
  pf = (void *)ROUNDUP(heap.start, PGSIZE);
  Log("free physical pages starting from %p", pf);

#ifdef HAS_VME
  vme_init(pg_alloc, free_page);
#endif
}
