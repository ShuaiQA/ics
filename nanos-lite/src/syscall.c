#include "syscall.h"
#include <common.h>

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;

  switch (a[0]) {
  case EVENT_YIELD:
    printf("yield\n");
    break;
  case EVENT_NULL:
    printf("gpr2 is [%d]\n", c->GPR2);
    halt(c->GPR2);
    break;
  default:
    panic("Unhandled syscall ID = %d", a[0]);
  }
}
