#include <fs.h>
#include <proc.h>

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

// 创建内核线程,其中线程的栈空间是由pcb构成的,传递相关的入口地址和参数arg(寄存器a0中)
Context *context_kload(PCB *pcb, void (*entry)(void *), void *arg) {
  Area area = {.start = pcb->stack, .end = pcb->stack + STACK_SIZE};
  pcb->cp = kcontext(area, entry, arg);
  return pcb->cp;
}

void init_proc() {
  context_kload(&pcb[0], hello_fun, (void *)10);
  switch_boot_pcb();
  Log("Initializing processes...");
  // naive_uload(NULL, "/bin/nterm");

  // load program here
}

Context *schedule(Context *prev) {
  // 将当前的上下文保存到current指向的pcb数组下标中
  current->cp = prev;
  // current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
  current = &pcb[0];
  return current->cp;
}
