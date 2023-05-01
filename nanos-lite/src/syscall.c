#include "syscall.h"
#include "am.h"
#include "klib-macros.h"
#include <common.h>

void SYS_yield(Context *c) {
  printf("yield\n");
  // yield();   // 直接调用yield不是会无限循环,调用ecall指令啊
  c->GPRx = 0; // 设置GPRx的返回值
}

int SYS_write(int fd, char *buf, size_t count) {
  if (fd == 1 || fd == 2) { // 代表的是stdout,stderr,输出到串口中
    for (size_t i = 0; i < count; i++) {
      putch(buf[i]);
    }
  }
  return count;
}

void SYS_exit(Context *c) { halt(c->GPR2); }

int SYS_brk(uint32_t size) {
  malloc(size);
  return 0;
}

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
  case EVENT_BRK:
    SYS_write(1, "aaaaa", 5);
    c->GPRx = SYS_brk(c->GPR2);
    break;
  case EVENT_WRITE:
    c->GPRx = SYS_write(c->GPR2, (char *)c->GPR3, c->GPR4);
    break;
  default:
    panic("Unhandled syscall ID = %d", a[0]);
  }
}
