#include "am.h"
#include "debug.h"
#include "klib-macros.h"
#include <common.h>
#include <fs.h>
#include <stdint.h>
#include <sys/time.h>

void sys_yield(Context *c) {
  Context *next = schedule(c);
  // 将返回的寄存器写到mscratch寄存器中
  asm volatile("csrw mscratch, %0" : : "r"(next));
}

int sys_gettimeofday(struct timeval *tv, struct timezone *tz) {
  AM_TIMER_UPTIME_T rtc;
  rtc = io_read(AM_TIMER_UPTIME);
  tv->tv_sec = rtc.us / 1000000;
  tv->tv_usec = rtc.us % 1000000;
  return 0;
}

void sys_exit() { naive_uload(NULL, "/bin/nterm"); }

uintptr_t sys_brk(uintptr_t size) { return 0; }

int sys_execve(const char *pathname, char *const argv[], char *const envp[]) {
  naive_uload(NULL, pathname);
  return 0;
}

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;

  switch (a[0]) {
  case SYS_yield:
    sys_yield(c);
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
  case SYS_gettimeofday:
    c->GPRx = sys_gettimeofday((struct timeval *)a[1], (struct timezone *)a[2]);
    break;
  case SYS_execve:
    c->GPRx = sys_execve((char *)a[1], (char **)a[2], (char **)a[3]);
    break;
  default:
    panic("Unhandled syscall ID = %d", a[0]);
  }
}
