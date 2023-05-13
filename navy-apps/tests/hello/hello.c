#include "syscall.h"
#include <stdio.h>
#include <unistd.h>

int main() {
  while (1) {
    _syscall_(SYS_yield, NULL, NULL);
  }

  // printf("a is %d address is %p\n", a, &a);
  // while (1) {
  //   write(1, "Hello World!\n", 13);
  // }
  // write(1, "Hello World!\n", 13);
  // int i = 2;
  // volatile int j = 0;
  // while (1) {
  //   j++;
  //   if (j == 10000) {
  //     printf("Hello World from Navy-apps for the %dth time!\n", i++);
  //     j = 0;
  //   }
  // }
  return 0;
}
