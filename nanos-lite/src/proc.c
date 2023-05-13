#include "am.h"
#include <proc.h>

#define MAX_NR_PROC 4

// 记录当前有多少个进程控制块
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

// 会调用kcontext()来创建上下文
// 并把返回的指针记录到PCB的cp中,对kcontext创建内核线程进行封装
Context *context_kload(PCB *pcb, void (*entry)(void *), void *arg) {
  pcb->cp = kcontext(NULL, entry, NULL);
  return pcb->cp;
}

void init_proc() {
  context_kload(&pcb[0], hello_fun, NULL);
  switch_boot_pcb();
  Log("Initializing processes...");

  naive_uload(NULL, "/bin/nterm");
  // load program here
}

Context *schedule(Context *prev) { return NULL; }
