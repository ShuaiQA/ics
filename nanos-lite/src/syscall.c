#include "am.h"
#include "klib-macros.h"
#include <common.h>
#include <fs.h>
#include <sys/time.h>

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime);

void SYS_yield(Context *c) {
  printf("yield\n");
  // yield();   // 直接调用yield不是会无限循环,调用ecall指令啊
  c->GPRx = 0; // 设置GPRx的返回值
}

void SYS_exit(Context *c) { halt(c->GPR2); }

void *SYS_brk(uint32_t size) { return (void *)malloc(size); }

int SYS_gettimeofday(struct timeval *tv, struct timezone *tz) {
  AM_TIMER_UPTIME_T rtc;
  __am_timer_uptime(&rtc);
  tv->tv_sec = rtc.us / 1000000;
  tv->tv_usec = rtc.us % 1000000;
  return 0;
}

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
    c->GPRx = fs_write(c->GPR2, (char *)c->GPR3, c->GPR4);
    break;
  case EVENT_BRK:
    c->GPRx = (uintptr_t)SYS_brk(c->GPR2);
    break;
  case EVENT_CLOSE:
    c->GPRx = fs_close(c->GPR2);
    break;
  case EVENT_LSEEK:
    c->GPRx = fs_lseek(c->GPR2, c->GPR3, c->GPR4);
    break;
  case EVENT_GETTIMEOFDAY:
    c->GPRx =
        SYS_gettimeofday((struct timeval *)c->GPR2, (struct timezone *)c->GPR3);
    break;
  default:
    panic("Unhandled syscall ID = %d", a[0]);
  }
}
