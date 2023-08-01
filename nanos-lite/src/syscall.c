#include "am.h"
#include "debug.h"
#include "klib-macros.h"
#include "proc.h"
#include <common.h>
#include <fs.h>
#include <stdint.h>
#include <sys/time.h>

// 首先需要确定的是ecall是否是可以进行step以及是否是yield
uintptr_t sys_yield(Context *c) { return (uintptr_t)schedule(c); }

int sys_gettimeofday(struct timeval *tv, struct timezone *tz) {
  AM_TIMER_UPTIME_T rtc;
  rtc = io_read(AM_TIMER_UPTIME);
  tv->tv_sec = rtc.us / 1000000;
  tv->tv_usec = rtc.us % 1000000;
  return 0;
}

uintptr_t sys_exit() {
  halt(0);
  return (uintptr_t)context_uload(current, "/bin/nterm", NULL, NULL);
}

// 传入的参数是新的地址,获取当前的进程的max_brk的地址空间
uintptr_t sys_brk(uintptr_t size) {
  uintptr_t sz = current->max_brk;
  assert(sz % PGSIZE == 0);
  Log("pre size is %p malloc size is %p",sz,size);
  if(size <= sz){ // 因为分配字节大小是4096个字节的所以新的大小可能会小于当前的大小
    return 0;
  }
  uintptr_t re = size - sz;
  uintptr_t mall_size = 0;
  while(re > 0){
    void *mem = new_page(1);
    re -= PGSIZE;
    mall_size += PGSIZE;
    map(&current->as, (char *)sz, mem, 0);
    sz += PGSIZE;
  }
  Log("change max_brk is %p",sz);
  current->max_brk = sz;
  return 0;
}

uintptr_t sys_execve(const char *pathname, char *const argv[],
                     char *const envp[]) {
  return (uintptr_t)context_uload(current, pathname, argv, envp);
}

Context *do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;

  switch (a[0]) {
  case SYS_exit:
    return (Context *)sys_exit();
  case SYS_yield:
    return (Context *)sys_yield(c);
  case SYS_open:
    c->GPRx = fs_open((char *)a[1], a[2], a[3]);
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
    return (Context *)sys_execve((char *)a[1], (char **)a[2], (char **)a[3]);
  default:
    panic("Unhandled syscall ID = %d", a[0]);
  }
  return c;
}
