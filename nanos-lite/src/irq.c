#include "am.h"
#include "arch/riscv32-nemu.h"
#include "syscall.h"
#include <debug.h>

static Context *do_event(Event e, Context *c) {
  do_syscall(c);
  return c;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  cte_init(do_event);
}
