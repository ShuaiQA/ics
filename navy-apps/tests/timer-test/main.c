#include <NDL.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>

const unsigned long Converter = 1000 * 1000; // 1s == 1000 * 1000 us

int main() {
  NDL_Init(0);
  int i = 1;
  while (1) {
    uint32_t next = NDL_GetTicks();
    if (next > 0.5 * Converter * i) {
      printf("Hello\n");
      i++;
    }
  }
  return 0;
}
