#include "am.h"
#include "arch/riscv32-nemu.h"
#include "syscall.h"
#include <common.h>

void sys_write(Context *c, int fd, void *buf, int count) {
  char *b = (char *)buf;
  int i = 0;
  for (; i < count; i++) {
    char t = *(b + i);
    putch(t);
  }
  c->GPRx = i;
}

static Context *do_event(Event e, Context *c) {
  do_syscall(c);
  return c;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  cte_init(do_event);
}
