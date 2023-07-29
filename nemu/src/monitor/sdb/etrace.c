// 如何识别嵌套的ecall
#include <assert.h>
#include <common.h>
#include <isa.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct etrace {
  word_t a0, a1, a2;
  int syscall;
  word_t ret;
} etrace;

#define SIZE_ETR_BUF 16
#define DEP 5
static etrace buf[SIZE_ETR_BUF][DEP];
static int step[SIZE_ETR_BUF]; // 记录每一个ecall嵌套的层数
static int max_step[SIZE_ETR_BUF];
static int pos = 0;

void new_etrace() {
  buf[pos][step[pos]].a0 = cpu.gpr[10];
  buf[pos][step[pos]].a1 = cpu.gpr[11];
  buf[pos][step[pos]].a2 = cpu.gpr[12];
  buf[pos][step[pos]].syscall = cpu.gpr[17];
  step[pos]++;
  max_step[pos]++;
}

void new_ret() {
  step[pos]--;
  buf[pos][step[pos]].ret = cpu.gpr[10];
  if (step[pos] == 0) {
    pos = (pos + 1) % SIZE_ETR_BUF;
  }
}

// 输出命令的调试信息
void print_etrace() {
  int i = pos;
  do {
    for (int j = 0; j < max_step[i]; j++) {
      for (int k = 0; k < j; k++) {
        printf("  ");
      }
      printf("syscall %d  parameter " FMT_WORD " " FMT_WORD "  " FMT_WORD
             " ret " FMT_WORD " \n",
             buf[i][j].syscall, buf[i][j].a0, buf[i][j].a1, buf[i][j].a2,
             buf[i][j].ret);
    }
    i = (i + 1) % SIZE_ETR_BUF;
  } while (i != pos);
}
