#ifndef ARCH_H__
#define ARCH_H__

struct Context {
  // 根据__am_asm_trap函数进行重组数据结构
  uintptr_t gpr[32], mcause, mstatus, mepc;
  void *pdir; // 指向的是当前进程的页目录地址
};

#define GPR1 gpr[17] // a7
#define GPR2 gpr[10]
#define GPR3 gpr[11]
#define GPR4 gpr[12]
#define GPRx gpr[10]

#endif
