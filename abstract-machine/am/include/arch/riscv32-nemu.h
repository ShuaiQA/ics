#ifndef ARCH_H__
#define ARCH_H__

struct Context {
  // 根据__am_asm_trap函数进行重组数据结构
  uintptr_t gpr[32], mcause, mstatus, mepc;
  void *pdir;
};

#define GPR1 gpr[17] // a7
#define GPR2 gpr[5]
#define GPR3 gpr[6]
#define GPR4 gpr[7]
#define GPRx gpr[5]

// intptr_t _syscall_(intptr_t type, intptr_t a0, intptr_t a1, intptr_t a2) {
//   register intptr_t _gpr1 asm (GPR1) = type;
//   register intptr_t _gpr2 asm (GPR2) = a0;
//   register intptr_t _gpr3 asm (GPR3) = a1;
//   register intptr_t _gpr4 asm (GPR4) = a2;
//   register intptr_t ret asm (GPRx);
//   asm volatile (SYSCALL : "=r" (ret) : "r"(_gpr1), "r"(_gpr2), "r"(_gpr3),
//   "r"(_gpr4)); return ret;
// }
#endif
