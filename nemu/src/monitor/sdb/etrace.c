// 设备访问环形缓冲区
#include "common.h"
#include "sdb.h"

typedef struct {
  word_t pc;
  word_t No;
} etrace;

#define SIZE_ETR_BUF 16
static etrace buf[SIZE_ETR_BUF];
static word_t pos = 0;

void new_etrace(word_t pc, word_t No) {
  buf[pos].pc = pc;
  buf[pos].No = No;
  pos = (pos + 1) % SIZE_ETR_BUF;
}

// 输出命令的调试信息
void print_etrace() {
  int i = pos;
  do {
    printf("pc [" FMT_WORD "] No [" FMT_WORD "]\n", buf[i].pc, buf[i].No);
    i = (i + 1) % SIZE_ETR_BUF;
  } while (i != pos);
}
