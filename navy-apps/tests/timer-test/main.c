#include <stdio.h>
#include <sys/time.h>

const unsigned long Converter = 1000 * 1000; // 1s == 1000 * 1000 us

int main() {
  struct timeval pre;
  int ret = gettimeofday(&pre, NULL);
  if (ret == -1) {
    printf("Error: gettimeofday()\n");
    return ret;
  }

  struct timeval next;
  while (ret != -1) {
    ret = gettimeofday(&next, NULL);
    unsigned long diff = (next.tv_sec * Converter + next.tv_usec) -
                         (pre.tv_sec * Converter + pre.tv_usec);
    if (diff > 0.5 * Converter) {
      printf("Hello\n");
      pre.tv_sec = next.tv_sec;
      pre.tv_usec = next.tv_usec;
    }
  }
  return 0;
}
