#include <stdint.h>
#include <syscall.h>

#ifdef __ISA_NATIVE__
#error can not support ISA=native
#endif

#define SYS_yield 1
extern int _syscall_(int, uintptr_t, uintptr_t, uintptr_t);

int main() {
  while (1) {
    char *buf = "aaa";
    _syscall_(SYS_write, 1, (uintptr_t)buf, 4);
  }
  return 0;
}
