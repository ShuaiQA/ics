#include "syscall.h"
#include "am.h"
#include "klib-macros.h"
#include <common.h>
#include <fs.h>
#include <proc.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime);

void sys_yield(Context *c) {
  // 首先确定的是寄存器sp指向的是一个上下文的结构体数据,然后根据该结构体数据进行恢复上下文
  // 我修改了__am_asm_trap汇编代码,先让sp寄存器先加载Context结构体中sp寄存器的位置
  // 然后在根据该位置进行恢复上下文即可完成相关的恢复工作
  c->gpr[2] = (uintptr_t)schedule(c);
}

void sys_exit(Context *c) { halt(c->GPR2); }

void *sys_brk(uint32_t size) { return (void *)malloc(size); }

int sys_gettimeofday(struct timeval *tv, struct timezone *tz) {
  AM_TIMER_UPTIME_T rtc;
  __am_timer_uptime(&rtc);
  tv->tv_sec = rtc.us / 1000000;
  tv->tv_usec = rtc.us % 1000000;
  return 0;
}

int execve(const char *pathname, char *const argv[], char *const envp[]) {
  naive_uload(NULL, pathname);
  return 0;
}

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;

  switch (a[0]) {
  case SYS_exit:
    sys_exit(c);
    break;
  case SYS_yield:
    sys_yield(c);
    break;
  case SYS_open:
    c->GPRx = fs_open((char *)c->GPR2, c->GPR3, c->GPR4);
    break;
  case SYS_read:
    c->GPRx = fs_read(c->GPR2, (void *)c->GPR3, c->GPR4);
    break;
  case SYS_execve:
    c->GPRx = execve((char *)c->GPR2, NULL, NULL);
    break;
  case SYS_write:
    printf("cur Context add is %p\n", c);
    c->GPRx = fs_write(c->GPR2, (char *)c->GPR3, c->GPR4);
    break;
  case SYS_brk:
    c->GPRx = (uintptr_t)sys_brk(c->GPR2);
    break;
  case SYS_close:
    c->GPRx = fs_close(c->GPR2);
    break;
  case SYS_lseek:
    c->GPRx = fs_lseek(c->GPR2, c->GPR3, c->GPR4);
    break;
  case SYS_gettimeofday:
    c->GPRx =
        sys_gettimeofday((struct timeval *)c->GPR2, (struct timezone *)c->GPR3);
    break;
  default:
    panic("Unhandled syscall ID = %d", a[0]);
  }
}
