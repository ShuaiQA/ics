#include <NDL.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>

const unsigned long Converter = 1000 * 1000; // 1s == 1000 * 1000 us

int main() {
  uint32_t pre = NDL_GetTicks();
  while (1) {
    uint32_t next = NDL_GetTicks();
    if (next - pre > 0.5 * Converter) {
      printf("Hello\n");
      pre = next;
    }
  }
  return 0;
}
