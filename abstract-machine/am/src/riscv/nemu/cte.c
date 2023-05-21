#include <am.h>
#include <klib.h>

static Context *(*user_handler)(Event, Context *) = NULL;
void __am_get_cur_as(Context *c);
void __am_switch(Context *c);

Context *__am_irq_handle(Context *c) {
  // 来将当前的地址空间描述符指针保存到上下文中
  __am_get_cur_as(c);
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

// 设置异常入口地址,注册一个事件处理回调函数(由操作系统提供)
bool cte_init(Context *(*handler)(Event, Context *)) {
  // initialize exception entry
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));
  user_handler = handler;
  return true;
}

// 创建内核线程,设置线程的上下文,初始化上下文中的pc,sp,a0寄存器
// (分别代表了分别代表了指令开始执行位置,栈指针,参数)
Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
  Context *ans = (Context *)kstack.end - 1;
  ans->mepc = (uintptr_t)entry;
  ans->gpr[2] = (uintptr_t)ans;
  ans->GPR2 = (uintptr_t)arg;
  return ans;
}

// yield的a7应该是1吧
void yield() { asm volatile("li a7, 1; ecall"); }

bool ienabled() { return false; }

void iset(bool enable) {}
