#include "am.h"
#include <proc.h>

#define MAX_NR_PROC 4

// 记录当前有多少个进程控制块
static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
// 当前的pcb_boot代表的是当前的Nanos进程
static PCB pcb_boot = {};
PCB *current = NULL;

void switch_boot_pcb() { current = &pcb_boot; }

// 很自然调用该函数的参数就是a0寄存器,在初始化上下文线程的时候将寄存器a0设置为arg
// 执行的时候就会取出相应的值
void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite with arg '%p' for the %dth time!",
        (uintptr_t)arg, j);
    j++;
    yield();
  }
}

// 会调用kcontext()来创建上下文
// 并把返回的指针记录到PCB的cp中,对kcontext创建内核线程进行封装
Context *context_kload(PCB *pcb, void (*entry)(void *), void *arg) {
  // 创建内核线程的栈空间
  Area area = {.start = pcb->stack, .end = pcb->stack + STACK_SIZE};
  pcb->cp = kcontext(area, entry, arg);
  return pcb->cp;
}

Context *context_uload(PCB *pcb, char *pathname) {
  void *entry = naive_uload(pcb, pathname);
  printf("%p\n", entry);
  Area area = {.start = pcb->stack, .end = pcb->stack + STACK_SIZE};
  pcb->cp = ucontext(NULL, area, entry);
  return pcb->cp;
}

void init_proc() {
  int a = 0x10000;
  context_kload(&pcb[0], hello_fun, (void *)a);
  context_uload(&pcb[1], "/bin/bird");
  switch_boot_pcb();
  Log("Initializing processes...");

  // load program here
  // naive_uload(NULL, "/bin/nterm");
}

Context *schedule(Context *prev) {
  // 将当前的上下文保存到current指向的pcb数组下标中
  current->cp = prev;
  current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
  return current->cp;
}

// 首先应该分析的是创建一个内核线程需要进行什么初始化操作
// 在最开始的时候需要确定的是上下文的mepc值,还有就是栈的位置
// 之后我们分析hello_fun来逐步的进行扩展到其余的情况
// 当我们初始化之后mepc和sp寄存器之后,函数就会正常执行
// 之后(进程执行一段时间)再进行yield的时候会发现当前的系统调用会重新保存在PCB中的cp指针中
// 由schedule可以发现,该cp会指向该进程里的一个新的Context由__am_asm_trap新创建的
// 之后获取新的pcb数组,然后返回一个Context上下文,执行别的线程流
// 上面介绍了线程的Context的初始化的过程,以及线程运行中Context的更新过程,选择下一个上下文更新的过程
