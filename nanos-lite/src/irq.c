#include "am.h"
#include "arch/riscv32-nemu.h"
#include <common.h>
#include <stdio.h>

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
  switch (e.event) {
  case EVENT_YIELD:
    printf("yield\n");
    break;
  case EVENT_NULL:
    halt(0);
    break;
  case EVENT_WRITE:
    sys_write(c, c->GPR2, (void *)c->GPR3, c->GPR4);
    break;
  default:
    panic("Unhandled event ID = %d", e.event);
  }

  return c;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  cte_init(do_event);
}
