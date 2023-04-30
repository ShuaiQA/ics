#include "syscall.h"
#include "am.h"
#include "klib-macros.h"
#include <common.h>

void SYS_yield(Context *c) {
  printf("yield\n");
  // yield();   // 直接调用yield不是会无限循环,调用ecall指令啊
  c->GPRx = 0; // 设置GPRx的返回值
  printf("nanos %d  %d  %p  %d %d\n", c->GPR1, c->GPR2, c->GPR3, c->GPR4,
         c->GPRx);
}

void SYS_write(Context *c) {
  uint32_t arg0 = c->GPR2, arg1 = c->GPR3, arg2 = c->GPR4;
  if (arg0 == 1 || arg0 == 2) { // 代表的是stdout,stderr,输出到串口中
    for (int i = 0; i < arg2; i++) {
      char c = *((char *)arg1 + i);
      putch(c);
    }
  }
  c->GPRx = arg2;
  printf("nanos %d  %d  %p  %d %d\n", c->GPR1, c->GPR2, c->GPR3, c->GPR4,
         c->GPRx);
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
  case EVENT_WRITE:
    SYS_write(c);
    break;
  default:
    panic("Unhandled syscall ID = %d", a[0]);
  }
}
