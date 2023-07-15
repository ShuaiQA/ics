#include "syscall.h"
#include "am.h"
#include "debug.h"
#include <common.h>
void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;

  switch (a[0]) {
  case SYS_yield:
    Log("get yield");
    break;
  default:
    panic("Unhandled syscall ID = %d", a[0]);
  }
}
