#include "debug.h"
#include <stdint.h>

struct {
  uintptr_t no, a0, a1, a2, ret;
} trace;

void ptb(uintptr_t a0, uintptr_t a1, uintptr_t a2, uintptr_t no) {
  trace.no = no;
  trace.a0 = a0;
  trace.a1 = a1;
  trace.a2 = a2;
}

void pte(uintptr_t ret) {
  trace.ret = ret;
  Log("%p  %p  %p  %p  %p", trace.no, trace.a0, trace.a1, trace.a2, trace.ret);
}
