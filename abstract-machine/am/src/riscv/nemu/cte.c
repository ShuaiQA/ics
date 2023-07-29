#include <am.h>
#include <klib.h>
#include <riscv/riscv.h>
#include <stdio.h>

static Context *(*user_handler)(Event, Context *) = NULL;

Context *__am_irq_handle(Context *c) {
  if (user_handler) {
    Event ev = {0};
    switch (c->mcause) {
    case 0xb:
      ev.event = EVENT_SYSCALL;
      c->mepc += 4;
      printf("mepc + 4 %p\n", c->mepc);
      break;
    default:
      ev.event = EVENT_ERROR;
      break;
    }
    c = user_handler(ev, c);
    assert(c != NULL);
  }

  return c;
}

extern void __am_asm_trap(void);

bool cte_init(Context *(*handler)(Event, Context *)) {
  // initialize exception entry
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

  // register event handler
  user_handler = handler;

  return true;
}

// 创建内核线程,设置线程的上下文,初始化上下文中的pc,sp,a0寄存器
// (分别代表了分别代表了指令开始执行位置,栈指针,参数)
Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
  Context *ctx = (Context *)kstack.end - 1;
  ctx->mepc = (uintptr_t)entry;
  ctx->gpr[2] = (uintptr_t)ctx;
  ctx->GPRx = (uintptr_t)arg;
  return ctx;
}

// 为了更加符合syscall将其修改为1
void yield() { asm volatile("li a7, 1; ecall"); }

bool ienabled() { return false; }

void iset(bool enable) {}
