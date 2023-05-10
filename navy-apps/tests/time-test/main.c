#include <am.h>
#include <stdio.h>
#include <unistd.h>

void rtc_test() {
  int sec = 1;
  while (1) {
    while ((io_read(AM_TIMER_UPTIME).us) / 1000000 < sec)
      ;
    printf("hello\n");
    sec++;
  }
}

int main() {
  ioe_init();
  rtc_test();
  return 0;
}
