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

// 获取AM的堆空间,使用AM的堆空间的最后位置(heap.end) 作为用户栈
// 因为栈是向低地址扩充不会超出内存范围,并且使用的是堆最后面的内存部分,暂时不会与操作系统的malloc进行冲突
extern Area heap;

// 返回新的上下文
Context *schedule(Context *prev);

size_t get_ramdisk_size();
size_t ramdisk_read(void *buf, size_t offset, size_t len);

void *naive_uload(PCB *pcb, const char *filename);
Context *context_uload(PCB *pcb, const char *pathname, char *const argv[],
                       char *const envp[]);

void switch_boot_pcb();

#endif
