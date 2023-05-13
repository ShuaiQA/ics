#ifndef __PROC_H__
#define __PROC_H__

#include <common.h>
#include <memory.h>

#define STACK_SIZE (8 * PGSIZE)

// 这是为每一个子进程创建的栈空间,开始的部分保存相关的内容
// 结束的部分用于保存每一个进程的上下文结构
typedef union {
  uint8_t stack[STACK_SIZE] PG_ALIGN;
  struct {
    Context *cp;
    AddrSpace as;
    // we do not free memory, so use `max_brk' to determine when to call _map()
    uintptr_t max_brk;
  };
} PCB;

extern PCB *current;

// 返回新的上下文
Context *schedule(Context *prev);

size_t get_ramdisk_size();
size_t ramdisk_read(void *buf, size_t offset, size_t len);

void *naive_uload(PCB *pcb, const char *filename);

#endif
