#include <klib-macros.h>
#include <klib.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t len = 0;
  for (; *s != '\0'; s++) {
    len++;
  }
  return len;
}

// 返回的是一个dst最初的位置
char *strcpy(char *dst, const char *src) {
  char *p = dst;
  while (*src != '\0') {
    *p++ = *src++;
  }
  *p = '\0';
  return dst;
}

// 返回的是一个dst最初的位置
char *strncpy(char *dst, const char *src, size_t n) {
  char *p = dst;
  for (size_t i = 0; i < n; i++) {
    *p++ = *src++;
  }
  return dst;
}

// 把 src 所指向的字符串追加到 dest 所指向的字符串的结尾
char *strcat(char *dst, const char *src) {
  char *p = dst;
  p = p + strlen(p);
  strcpy(p, src);
  return dst;
}

int strcmp(const char *s1, const char *s2) {
  while (*s1 != '\0' && *s2 != '\0') {
    if (*s1 > *s2) {
      return 1;
    } else if (*s1 < *s2) {
      return -1;
    }
    s1++;
    s2++;
  }
  if (*s1 == '\0' && *s2 == '\0') {
    return 0;
  } else if (*s1 == '\0') {
    return -1;
  } else {
    return 1;
  }
}

// 比较到\0就结束了
int strncmp(const char *s1, const char *s2, size_t n) {
  for (int i = 0; i < n; i++) {
    if (*s1 == '\0' || *s2 == '\0') {
      break;
    }
    if (*s1 > *s2) {
      return 1;
    } else if (*s1 < *s2) {
      return -1;
    }
    s1++;
    s2++;
  }
  if (*s1 == '\0' && *s2 == '\0') {
    return 0;
  } else if (*s1 == '\0') {
    return -1;
  } else {
    return 1;
  }
}

void *memset(void *s, int c, size_t n) {
  unsigned char *p = (unsigned char *)s;
  while (n > 0) {
    *p++ = (unsigned char)c;
    n--;
  }
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  unsigned char *d = (unsigned char *)dst;
  unsigned char *s = (unsigned char *)src;
  if (s + n > d && d > s) { // 发生重叠反向拷贝
    while (n--) {
      *(d + n) = *(s + n);
    }
  } else {
    while (n--) {
      *d++ = *s++;
    }
  }
  return dst;
}

// 不支持in和out有重叠区域
void *memcpy(void *out, const void *in, size_t n) {
  unsigned char *o = (unsigned char *)out;
  unsigned char *i = (unsigned char *)in;
  for (size_t ii = 0; ii < n; ii++) {
    *o++ = *i++;
  }
  return out;
}

// 按照n个进行比较即使是到\0
int memcmp(const void *s1, const void *s2, size_t n) {
  unsigned char *b1 = (unsigned char *)s1;
  unsigned char *b2 = (unsigned char *)s2;
  while (n--) {
    if (*b1 > *b2) {
      return 1;
    } else if (*b1 < *b2) {
      return -1;
    }
    b1++;
    b2++;
  }
  return 0;
}

#endif
