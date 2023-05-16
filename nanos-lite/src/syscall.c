#include "syscall.h"
#include "am.h"
#include "debug.h"
#include "klib-macros.h"
#include <common.h>
#include <fs.h>
#include <proc.h>
#include <sys/time.h>

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime);

// 首先确定的是寄存器sp指向的是一个上下文的结构体数据,然后根据该结构体数据进行恢复上下文
// 我修改了__am_asm_trap汇编代码,先让sp寄存器先加载Context结构体中a0寄存器的位置
// 然后在根据该位置进行恢复上下文即可完成相关的恢复工作

void sys_yield(Context *c) {
  uintptr_t sp = (uintptr_t)schedule(c);
  Log("sp is %p", sp);
  asm volatile("mv sp, %0" : : "r"(sp));
}

void sys_exit(Context *c) { halt(c->GPR2); }

// 判断当前的addr是否和其余的程序有冲突
void *sys_brk(uint32_t addr) { return 0; }

int sys_gettimeofday(struct timeval *tv, struct timezone *tz) {
  AM_TIMER_UPTIME_T rtc;
  __am_timer_uptime(&rtc);
  tv->tv_sec = rtc.us / 1000000;
  tv->tv_usec = rtc.us % 1000000;
  return 0;
}

void sys_execve(const char *pathname, char *const argv[], char *const envp[]) {
  current->cp = context_uload(current, pathname, argv, envp);
}

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  // Log("GPR1 is %d\n", a[0]);
  // Log("%p\n", c);

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
    // c->GPRx不再接受返回值,防止覆盖context_uload里设置的内容
    sys_execve((char *)c->GPR2, (char **)c->GPR3, (char **)c->GPR4);
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
