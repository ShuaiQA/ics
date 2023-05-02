#include "sdb.h"

typedef struct isa {
  word_t isa_val;
  word_t pc;
  char action[50];
} ISAbuf;

#define SIZE_ISA_BUF 16
static ISAbuf buf[SIZE_ISA_BUF];
static int pos = 0;

void print_isa(word_t n, int cnt) {
  if (cnt < 31) {
    print_isa(n / 2, cnt + 1);
    if (cnt == 24 || cnt == 19 || cnt == 14 || cnt == 11 || cnt == 6) {
      printf(" ");
    }
  }
  printf("%d", n % 2); /*输出二进制数值*/
}

void new_isa(word_t pc, word_t isa_val) {
  buf[pos].pc = pc;
  buf[pos].isa_val = isa_val;
  void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
  disassemble(buf[pos].action, 50, MUXDEF(CONFIG_ISA_x86, pc + 4, pc),
              (uint8_t *)&isa_val, 4);

  pos = (pos + 1) % SIZE_ISA_BUF;
}

// 输出命令的调试信息
void printIringBuf() {
  int i = pos;
  do {
    printf("[0x%08x]: %s\t    ", buf[i].pc, buf[i].action);
    print_isa(buf[i].isa_val, 0);
    printf("\n");
    i = (i + 1) % SIZE_ISA_BUF;
  } while (i != pos);
}
