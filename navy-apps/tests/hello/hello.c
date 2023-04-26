#include <stdint.h>
#include <stdio.h>

#ifdef __ISA_NATIVE__
#error can not support ISA=native
#endif

extern int _write(int fd, void *buf, int count);

int main() {
  char *a = "hello";
  printf("%p\n", a);
  return _write(1, a, 5);
}
// #include <stdio.h>
// #include <unistd.h>
//
// int main() {
//   write(1, "Hello World!\n", 13);
//   int i = 2;
//   volatile int j = 0;
//   while (1) {
//     j++;
//     if (j == 10000) {
//       printf("Hello World from Navy-apps for the %dth time!\n", i++);
//       j = 0;
//     }
//   }
//   return 0;
// }
