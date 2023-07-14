#include "common.h"
#include "sdb.h"

typedef struct isa {
  char logbuf[128];
} ISAbuf;

#define SIZE_ISA_BUF 16
static ISAbuf buf[SIZE_ISA_BUF];
static word_t pos = 0;

void new_isa(char *isa) {
  memcpy(buf[pos].logbuf, isa, strlen(isa));
  pos = (pos + 1) % SIZE_ISA_BUF;
}

// 输出命令的调试信息
void printIringBuf() {
  int i = pos;
  do {
    printf("%s\n", buf[i].logbuf);
    i = (i + 1) % SIZE_ISA_BUF;
  } while (i != pos);
}
