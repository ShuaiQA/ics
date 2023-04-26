#include <am.h>
#include <klib-macros.h>
#include <klib.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char buf[10240];
  int n = vsnprintf(buf, 10240, fmt, args);
  va_end(args);
  for (int i = 0; buf[i] != '\0'; i++) {
    putch(buf[i]);
  }
  return n;
}

int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int num = vsnprintf(out, 100000, fmt, ap);
  va_end(ap);
  return num;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  int num = vsnprintf(out, 100000, fmt, ap);
  return num;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int num = vsnprintf(out, n, fmt, ap);
  va_end(ap);
  return num;
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  int nn = 0;
  int num = 0;
  uint32_t temp;
  char *a;
  char t[32];
  int tt = 0;
  int i = 0;
  while (n-- && *fmt) {
    if (*fmt != '%') {
      *out++ = *fmt++;
      num++;
      continue;
    }
    // 当前是%
    switch (*++fmt) {
    case 's':
      a = va_arg(ap, char *);
      for (int i = 0; a[i] != '\0'; i++) {
        num++;
        *out++ = a[i];
      }
      tt = 1; // 标记当前%s
      break;
    case 'x':
    case 'p':
      temp = va_arg(ap, uint32_t);
      i = 0;
      *out++ = '0';
      *out++ = 'x';
      while (temp != 0) {
        int yu = temp % 16;
        if (yu < 10) {
          t[i] = '0' + yu;
        } else {
          t[i] = 'a' + yu - 10;
        }
        temp /= 16;
        i++;
      }
      if (i == 0) {
        *out++ = '0';
      }
      for (int j = i - 1; j >= 0; j--) {
        *out++ = t[j];
        num++;
      }
      tt = 1;
      break;
    case 'd':
      nn = va_arg(ap, int);
      if (nn < 0) {
        *out++ = '-';
        nn = -nn;
      }
      i = 0;
      while (nn != 0) {
        t[i] = '0' + (nn % 10);
        nn /= 10;
        i++;
      }
      if (i == 0) {
        *out++ = '0';
      }
      for (int j = i - 1; j >= 0; j--) {
        *out++ = t[j];
        num++;
      }
      tt = 1;
      break;
    }
    if (tt == 0) { // 当前不是
      *out++ = '%';
      *out++ = *fmt;
      num += 2;
    }
    fmt++;
  }
  *out = '\0';
  return num;
}

#endif
