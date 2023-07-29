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
  struct etrace *next; // 记录嵌套的层数
} etrace;

#define SIZE_ETR_BUF 16
static etrace buf[SIZE_ETR_BUF];
static int step[SIZE_ETR_BUF]; // 记录每一个ecall嵌套的层数
static int pos = 0;

void new_etrace() {
  if (step[pos] >= 1) {
    etrace *cur = malloc(sizeof(etrace));
    cur->a0 = cpu.gpr[10];
    cur->a1 = cpu.gpr[11];
    cur->a2 = cpu.gpr[12];
    cur->syscall = cpu.gpr[17];
    etrace *last = &buf[pos];
    while (last->next != NULL) {
      last = last->next;
    }
    last->next = cur;
  } else {
    buf[pos].a0 = cpu.gpr[10];
    buf[pos].a1 = cpu.gpr[11];
    buf[pos].a2 = cpu.gpr[12];
    buf[pos].syscall = cpu.gpr[17];
  }
  step[pos]++;
}

void new_ret() {
  int cur_step = step[pos];
  if (cur_step == 1) {
    buf[pos].ret = cpu.gpr[10];
    step[pos]--;
    assert(step[pos] == 0); // 跳转到下一个则上一个必定完毕
    pos = (pos + 1) % SIZE_ETR_BUF;
  } else {
    etrace *p = &buf[pos];
    while (cur_step > 1) {
      p = p->next;
      cur_step--;
    }
    p->ret = cpu.gpr[10];
    step[pos]--;
  }
}

// 输出命令的调试信息
void print_etrace() {
  int i = pos;
  do {
    etrace *cur = &buf[i];
    int step = 0;
    while (cur != NULL) {
      for (int i = 0; i < step; i++) {
        printf("  ");
      }
      printf("syscall %d  parameter " FMT_WORD " " FMT_WORD "  " FMT_WORD
             " ret " FMT_WORD " \n",
             cur->syscall, cur->a0, cur->a1, cur->a2, cur->ret);
      cur = cur->next;
    }
    i = (i + 1) % SIZE_ETR_BUF;
  } while (i != pos);
}
