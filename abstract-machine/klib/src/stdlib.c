#include <am.h>
#include <klib-macros.h>
#include <klib.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
static unsigned long int next = 1;

int rand(void) {
  // RAND_MAX assumed to be 32767
  next = next * 1103515245 + 12345;
  return (unsigned int)(next / 65536) % 32768;
}

void srand(unsigned int seed) { next = seed; }

int abs(int x) { return (x < 0 ? -x : x); }

int atoi(const char *nptr) {
  int x = 0;
  while (*nptr == ' ') {
    nptr++;
  }
  while (*nptr >= '0' && *nptr <= '9') {
    x = x * 10 + *nptr - '0';
    nptr++;
  }
  return x;
}

static char *mem_heap;
static char *mem_brk;

void *malloc(size_t size) {
  // On native, malloc() will be called during initializaion of C runtime.
  // Therefore do not call panic() here, else it will yield a dead recursion:
  //   panic() -> putchar() -> (glibc) -> malloc() -> panic()
  // #if !(defined(__ISA_NATIVE__) && defined(__NATIVE_USE_KLIB__))
  //   panic("Not implemented");
  // #endif
  if (mem_heap == NULL) {
    mem_heap = (char *)ROUNDUP(heap.start, 8);
    mem_brk = mem_heap;
    return (void *)mem_brk;
  }
  char *old_brk = mem_brk;
  mem_brk += size;
  return (void *)old_brk;
}

void free(void *ptr) {}

#endif
