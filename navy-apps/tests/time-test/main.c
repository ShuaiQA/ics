#include <am.h>
#include <stdio.h>
#include <unistd.h>

void rtc_test() {
  AM_TIMER_UPTIME_T o = io_read(AM_TIMER_UPTIME);
  int sec = 1;
  while (1) {
    while ((io_read(AM_TIMER_UPTIME).us - o.us) / 1000000 < sec)
      ;
    printf("hello\n");
    sec++;
  }
}

int main() {
  rtc_test();
  return 0;
}
