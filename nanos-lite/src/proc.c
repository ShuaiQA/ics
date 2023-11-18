#include "debug.h"
#include "memory.h"
#include <fs.h>
#include <proc.h>
#include <stdint.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void switch_boot_pcb() { current = &pcb_boot; }

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite with arg '%p' for the %dth time!",
        (uintptr_t)arg, j);
    j++;
    yield();
  }
}

void sleep(int time) {
  AM_TIMER_UPTIME_T rtc = io_read(AM_TIMER_UPTIME);
  do {
    AM_TIMER_UPTIME_T r = io_read(AM_TIMER_UPTIME);
    if (r.us - rtc.us > time * 1000000) {
      break;
    }
  } while (1);
}

// 创建内核线程,其中线程的栈空间是由pcb构成的,传递相关的入口地址和参数arg(寄存器a0中)
Context *context_kload(PCB *pcb, void (*entry)(void *), void *arg) {
  Area area = {.start = pcb->stack, .end = pcb->stack + STACK_SIZE};
  pcb->cp = kcontext(area, entry, arg);
  return pcb->cp;
}

// buf提供了一个地址空间,将argv参数放到buf地址空间的下面
uintptr_t setArgv(char *buf, char *const argv[]) {
  int del = 0;
  int i = 0;
  while (argv != NULL && argv[i] != NULL) {
    del += 1;
    *(buf - del) = '\0';
    size_t size = strlen(argv[i]);
    del += size;
    memcpy(buf - del, argv[i], size);
    i++;
  }
  del += 4;
  *(int *)(buf - del) = i;
  return del;
}

// 创建用户进程需要进行初始化有:1.在ucontext设置pc值,2.在当前暂时保存栈空间到a0寄存器中
// 创建用户进程,首先是找到解析elf文件获取entry,设置用户进程的栈空间
Context *context_uload(PCB *pcb, const char *pathname, char *const argv[],
                       char *const envp[]) {
  protect(&pcb->as); // 用户初始化页目录
  uintptr_t entry = naive_uload(pcb, pathname);
  Log("create pagetable %p, user begin pc is %p", pcb->as.ptr, entry);
  Area area = {.start = pcb->stack, .end = pcb->stack + STACK_SIZE};
  pcb->cp = ucontext(&pcb->as, area, (void *)entry);
  // 用户程序的约定,先将栈指针放到寄存器a0上,在用户空间初始的_start上在进行将a0转移到sp寄存器上
  pcb->cp->GPRx = 0x80000000 - sizeof(Context);
  return pcb->cp;
}

void init_proc() {
  context_uload(&pcb[0], "/bin/nterm", NULL, NULL);
  /* context_kload(&pcb[1], hello_fun, NULL); */
  /* context_uload(&pcb[1], "/bin/hello", NULL, NULL); */
  switch_boot_pcb();
  Log("Initializing processes... %p ", pcb);
  // load program here
}

Context *schedule(Context *prev) {
  // 将当前的上下文保存到current指向的pcb数组下标中
  current->cp = prev;
  /* if (current == &pcb[0]) { */
  /*   current = &pcb[1]; */
  /*   Log("schedule 1"); */
  /* } else { */
  /*   current = &pcb[0]; */
  /*   Log("schedule 0"); */
  /* } */
  Log("schedule 0");
  current = &pcb[0];
  return current->cp;
}
