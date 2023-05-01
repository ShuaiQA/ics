#include "am.h"
#include "klib-macros.h"
#include <common.h>
#include <fs.h>

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
  } else {
    fs_write(fd, buf, count);
  }
  return count;
}

void SYS_exit(Context *c) { halt(c->GPR2); }

void *SYS_brk(uint32_t size) { return (void *)malloc(size); }

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;

  switch (a[0]) {
  case EVENT_NULL:
    SYS_exit(c);
    break;
  case EVENT_YIELD:
    SYS_yield(c);
    break;
  case EVENT_OPEN:
    c->GPRx = fs_open((char *)c->GPR2, c->GPR3, c->GPR4);
    break;
  case EVENT_READ:
    c->GPRx = fs_read(c->GPR2, (void *)c->GPR3, c->GPR4);
    break;
  case EVENT_WRITE:
    c->GPRx = SYS_write(c->GPR2, (char *)c->GPR3, c->GPR4);
    break;
  case EVENT_BRK:
    c->GPRx = (uint32_t)SYS_brk(c->GPR2);
    break;
  case EVENT_LSEEK:
    c->GPRx = fs_lseek(c->GPR2, c->GPR3, c->GPR4);
    break;
  default:
    panic("Unhandled syscall ID = %d", a[0]);
  }
}
