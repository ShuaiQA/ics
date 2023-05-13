#include "syscall.h"
#include "am.h"
#include "klib-macros.h"
#include <common.h>
#include <fs.h>
#include <proc.h>
#include <stdint.h>
#include <sys/time.h>

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime);

// 当前的Context是保存的上一个上下文,根据__am_asm_trap汇编代码分析后
// 上下文的恢复是根据寄存器sp进行恢复的,我们只需要修改sp寄存器指向其余的上下文
// 那么就能够跳转到其余的进程了
void sys_yield(Context *c) {
  printf("yield\n");
  Context *next = schedule(c);
  // 修改sp的值(我们应该将更新sp寄存器的值添加到汇编代码中)
  // 这样之后上下文恢复就是根据新的sp的上下文进行恢复的
  c->gpr[2] = (uintptr_t)next;
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
