#include <stdint.h>

#ifdef __ISA_NATIVE__
#error can not support ISA=native
#endif

#define SYS_yield 1
extern int _syscall_(int, uintptr_t, uintptr_t, uintptr_t);

int main() {
  while (1) {
    char *buf = "aaa\n";
    _syscall_(4, 1, (uintptr_t)buf, 5);
  }
  return 0;
}
