// 设备访问环形缓冲区
#include "common.h"
#include "sdb.h"

typedef struct {
  word_t pc;
  word_t addr;
  char name[10];
} device;

#define SIZE_DEV_BUF 16
static device buf[SIZE_DEV_BUF];
static int pos = 0;

// 传入的内容是当前的pc和访问内存的地址
void new_device_trace(word_t pc, word_t addr) {
  buf[pos].pc = pc;
  buf[pos].addr = addr;

  pos = (pos + 1) % SIZE_DEV_BUF;
}

// 输出命令的调试信息
void print_device_trace() {
  int i = pos;
  do {
    i = (i + 1) % SIZE_DEV_BUF;
  } while (i != pos);
}
