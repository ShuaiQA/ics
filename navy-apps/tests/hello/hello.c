#include <stdio.h>
#include <unistd.h>

int main() {
  void *isp;
  __asm volatile("mov %0, sp" : "=r"(isp) : :);
  printf("%p\n", isp);
  write(1, "Hello World!\n", 13);
  int i = 2;
  volatile int j = 0;
  while (1) {
    j++;
    if (j == 10000) {
      printf("Hello World from Navy-apps for the %dth time!\n", i++);
      j = 0;
    }
  }
  return 0;
}
