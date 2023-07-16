#include "am.h"

#include "debug.h"
#include <common.h>
#include <fs.h>
#include <stdint.h>

uintptr_t sys_yield() {
  Log("get yield");
  return 0;
}

void sys_exit() { halt(0); }

uintptr_t sys_brk(uintptr_t size) { return 0; }

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;

  switch (a[0]) {
  case SYS_yield:
    c->GPRx = sys_yield();
    break;
  case SYS_open:
    c->GPRx = fs_open((char *)a[1], a[2], a[3]);
    break;
  case SYS_exit:
    sys_exit();
    break;
  case SYS_read:
    c->GPRx = fs_read(a[1], (char *)a[2], a[3]);
    break;
  case SYS_lseek:
    c->GPRx = fs_lseek(a[1], a[2], a[3]);
    break;
  case SYS_write:
    c->GPRx = fs_write(a[1], (void *)a[2], a[3]);
    break;
  case SYS_brk:
    c->GPRx = sys_brk(a[1]);
    break;
  case SYS_close:
    c->GPRx = fs_close(a[1]);
    break;
  default:
    panic("Unhandled syscall ID = %d", a[0]);
  }
}
