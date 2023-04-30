#include "syscall.h"
#include "am.h"
#include <common.h>

void SYS_yield(Context *c) {
  printf("yield\n");
  // yield();   // 直接调用yield不是会无限循环,调用ecall指令啊
  c->GPRx = 10;
}

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;

  switch (a[0]) {
  case EVENT_YIELD:
    SYS_yield(c);
    break;
  case EVENT_NULL:
    printf("gpr2 is [%d]\n", c->GPR2);
    halt(c->GPR2);
    break;
  default:
    panic("Unhandled syscall ID = %d", a[0]);
  }
}
