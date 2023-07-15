#include <klib.h>

#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  int n;

  for (n = 0; s[n]; n++)
    ;
  return n;
}

char *strcpy(char *s, const char *t) {
  char *os;

  os = s;
  while ((*s++ = *t++) != 0)
    ;
  return os;
}

char *strncpy(char *s, const char *t, size_t n) {
  char *os;

  os = s;
  while (n-- > 0 && (*s++ = *t++) != 0)
    ;
  while (n-- > 0)
    *s++ = 0;
  return os;
}

char *strcat(char *dst, const char *src) {
  char *os = dst;
  strcpy(dst + strlen(dst), src);
  return os;
}

int strcmp(const char *p, const char *q) {
  while (*p && *p == *q)
    p++, q++;
  return (char)*p - (char)*q;
}

int strncmp(const char *p, const char *q, size_t n) {
  while (*p && *p == *q && --n)
    p++, q++;
  return (char)*p - (char)*q;
}

void *memset(void *dst, int c, size_t n) {
  char *cdst = (char *)dst;
  int i;
  for (i = 0; i < n; i++) {
    cdst[i] = c;
  }
  return dst;
}

void *memmove(void *vdst, const void *vsrc, size_t n) {
  char *dst;
  const char *src;

  dst = vdst;
  src = vsrc;
  if (src > dst) {
    while (n-- > 0)
      *dst++ = *src++;
  } else {
    dst += n;
    src += n;
    while (n-- > 0)
      *--dst = *--src;
  }
  return vdst;
}

void *memcpy(void *dst, const void *src, size_t n) {
  return memmove(dst, src, n);
}

int memcmp(const void *s1, const void *s2, size_t n) {
  const char *p1 = s1, *p2 = s2;
  while (n-- > 0) {
    if (*p1 != *p2) {
      return *p1 - *p2;
    }
    p1++;
    p2++;
  }
  return 0;
}

#endif
