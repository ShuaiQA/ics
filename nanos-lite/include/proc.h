#ifndef __PROC_H__
#define __PROC_H__

#include <common.h>
#include <memory.h>
#include <stdint.h>

#define STACK_SIZE (8 * PGSIZE)

typedef union {
  uint8_t stack[STACK_SIZE] PG_ALIGN;
  struct {
    Context *cp;
    AddrSpace as;
    uintptr_t max_brk;
  };
} PCB;

extern PCB *current;

Context *schedule(Context *prev);

#endif
