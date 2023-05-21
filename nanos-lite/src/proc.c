#include "am.h"
#include "debug.h"
#include "memory.h"
#include <proc.h>
#include <stdint.h>

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
    if (j % 2000 == 0) {
      Log("Hello World from Nanos-lite with arg '%p' for the %dth time!",
          (uintptr_t)arg, j);
    }
    j++;
    yield();
  }
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
  protect(&pcb->as); // 每一个进程对AddrSpace进行初始化
  void *entry = naive_uload(pcb, pathname);
  Area area = {.start = pcb->stack, .end = pcb->stack + STACK_SIZE};
  pcb->cp = ucontext(&pcb->as, area, entry);
  // 用户程序的约定,先将栈指针放到寄存器a0上,在用户空间初始的_start上在进行将a0转移到sp寄存器上
  uintptr_t len = setArgv(area.end - sizeof(Context), argv);
  pcb->cp->GPRx = (uintptr_t)pcb->as.area.end - len - sizeof(Context);
  return pcb->cp;
}

void init_proc() {
  // context_kload(&pcb[0], hello_fun, NULL);
  // context_kload(&pcb[0], hello_fun, NULL);
  // context_uload(&pcb[1], "/bin/nterm", NULL, NULL);
  context_uload(&pcb[0], "/bin/dummy", NULL, NULL);
  switch_boot_pcb();
  Log("Initializing processes...");

  // load program here
  // void *entry = naive_uload(NULL, "/bin/menu");
  // ((void (*)())entry)();
}

// 会更新当前的cp指针指向相关的Context,主要的目的是为了下一次在访问该线程的时候直接调用PCB数组下标的cp就好了
// 返回根据相关的调度信息返回一个新的上下文,最后返回该上下文的指针
Context *schedule(Context *prev) {
  // 将当前的上下文保存到current指向的pcb数组下标中
  current->cp = prev;
  // current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
  current = &pcb[0];
  Log("next is pcb0");
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

// 分析为什么用户程序会发生系统调用的错误,首先是初始化用户程序,经过测试发现用户线程执行到系统调用之前并没有错误
// 在系统调用会发生错误(除了用户进程yield会执行正确,其余的系统调用都是错误的)
// 开始进行分析,用户进程在系统调用的过程中首先会保存相关的寄存器信息保存到自己的栈空间上面
// 在printf输出相关的Context指针地址我们也可以发现是这样的,之后就开始进行系统调用函数的处理
// 以write异常为例进行处理,在执行到异常的时候保存当前栈上的Context然后不断的进入相关的函数
// 直到对寄存器a0进行返回值的修改,根据指令的buf中发现在lw sp,8(sp)
// 之后就发生了错误,不断的定位发现当异常号不是yield的时候,不需要在次进行lw
// sp,8(sp) 换句话说说对于yield异常来说需要执行,其余的异常来说不需要执行
