#include <common.h>
#include <isa.h>

typedef struct {
  word_t a0, a1, a2;
  int syscall;
  word_t ret;
} etrace;

#define SIZE_ETR_BUF 16
static etrace buf[SIZE_ETR_BUF];
static int pos = 0;

void new_etrace() {
  buf[pos].a0 = cpu.gpr[10];
  buf[pos].a1 = cpu.gpr[11];
  buf[pos].a2 = cpu.gpr[12];
  buf[pos].syscall = cpu.gpr[17];
}

void new_ret() {
  buf[pos].ret = cpu.gpr[10];
  pos = (pos + 1) % SIZE_ETR_BUF;
}

// 输出命令的调试信息
void print_etrace() {
  int i = pos;
  do {
    printf("syscall %d  parameter " FMT_WORD " " FMT_WORD "  " FMT_WORD
           " ret " FMT_WORD " \n",
           buf[i].syscall, buf[i].a0, buf[i].a1, buf[i].a2, buf[i].ret);
    i = (i + 1) % SIZE_ETR_BUF;
  } while (i != pos);
}
