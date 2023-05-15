// 设备访问环形缓冲区
#include "common.h"
#include "sdb.h"

typedef struct {
  const char *name;
  word_t pc;
  word_t addr;
  word_t data;
} device;

#define SIZE_DEV_BUF 16
static device buf[SIZE_DEV_BUF];
static word_t pos = 0;

// 传入的内容是当前的pc和访问内存的地址
void new_device_trace(word_t pc, word_t addr, word_t data, const char *name) {
  buf[pos].pc = pc;
  buf[pos].addr = addr;
  buf[pos].name = name;
  buf[pos].data = data;
  pos = (pos + 1) % SIZE_DEV_BUF;
}

// 输出命令的调试信息
void print_device_trace() {
  int i = pos;
  do {
    printf("device name [%s] pc [" FMT_WORD "] addr [" FMT_PADDR
           "] data [" FMT_WORD "]\n",
           buf[i].name, buf[i].pc, buf[i].addr, buf[i].data);
    i = (i + 1) % SIZE_DEV_BUF;
  } while (i != pos);
}
