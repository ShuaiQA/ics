#include <am.h>
#include <klib.h>
#include <stdint.h>

static Context *(*user_handler)(Event, Context *) = NULL;

Context *__am_irq_handle(Context *c) {
  if (user_handler) {
    Event ev = {0};

    // 目前都是cpu自增4的
    c->mepc += 4;
    // 设置异常处理情况
    ev.event = c->GPR1;

    c = user_handler(ev, c);
    assert(c != NULL);
  }

  return c;
}

// 异常处理的入口函数地址,也就是ecall函数,之后的pc值的指向地址
// 查看汇编代码该函数的主要操作是,将32个寄存器的值保存到栈中
// 上述操作对t0,t1,t2寄存器保存完毕之后,
// 将寄存器中的mcause, mstatus, mepc再写到对应的t0,t1,t2寄存器中
// 将t0,t1,t2寄存器中的值在保存到栈中,跳转到__am_irq_handle函数
// 之后的操作就是将结构体中的mstatus,mepc数据更新到对应的t1和t2中
// t1,t2在更新到寄存器mstatus,mepc中,之后在恢复所有的寄存器的值
// 最后调用mret将mepc寄存器中的pc值放到pc寄存器中
extern void __am_asm_trap(void);

// 设置异常入口地址,注册一个事件处理回调函数
// 回调函数由Nanos-lite提供handler
bool cte_init(Context *(*handler)(Event, Context *)) {
  // initialize exception entry
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

  user_handler = handler;

  return true;
}

// kstack的end创建一个以entry为返回地址的上下文结构,将上下文指针进行返回
Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
  // 获取最后一个Context空间
  Context *ans = (Context *)kstack.end - 1;
  // 设置mepc也就是恢复上下文之后的pc值,以及sp寄存器的值(栈指针寄存器,用来分配局部变量)
  ans->mepc = (uintptr_t)entry;
  ans->gpr[2] = (uintptr_t)ans;
  // 使用寄存器a0进行参数传递
  ans->gpr[10] = (uintptr_t)arg;
  return ans;
}

// yield的a7应该是1吧
void yield() { asm volatile("li a7, 1; ecall"); }

bool ienabled() { return false; }

void iset(bool enable) {}
