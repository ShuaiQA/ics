#include "syscall.h"
#include "am.h"
#include <common.h>

void SYS_yield(Context *c) {
  printf("yield\n");
  // yield();   // 直接调用yield不是会无限循环,调用ecall指令啊
  c->GPRx = 0; // 设置GPRx的返回值
}

void SYS_exit(Context *c) { halt(c->GPR2); }

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;

  switch (a[0]) {
  case EVENT_YIELD:
    SYS_yield(c);
    break;
  case EVENT_NULL:
    SYS_exit(c);
    break;
  default:
    panic("Unhandled syscall ID = %d", a[0]);
  }
}
