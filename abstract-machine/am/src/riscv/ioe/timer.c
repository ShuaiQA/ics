#include <am.h>
#include <nemu.h>

AM_TIMER_UPTIME_T boot_time;

void __am_timer_init() {
  uint64_t n2 = inl(RTC_ADDR + 4);
  uint64_t n1 = inl(RTC_ADDR);
  boot_time.us = (n2 >> 32) + n1;
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  uint64_t n2 = inl(RTC_ADDR + 4);
  uint64_t n1 = inl(RTC_ADDR);
  uptime->us = (n2 >> 32) + n1 - boot_time.us;
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour = 0;
  rtc->day = 0;
  rtc->month = 0;
  rtc->year = 1900;
}
