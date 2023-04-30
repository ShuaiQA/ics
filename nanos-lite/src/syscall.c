#include "syscall.h"
#include "am.h"
#include <common.h>

void SYS_yield(Context *c) {
  printf("yield\n");
  // yield();
  c->GPRx = 0;
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
