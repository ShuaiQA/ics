#include <common.h>

typedef struct {
  uint32_t a7, a0, a1, a2;
} strace;

#define SIZE_STR_BUF 16
static strace buf[SIZE_STR_BUF];
static int pos = 0;

void new_etrace(int a7, int a0, int a1, int a2) {
  buf[pos].a0 = a0;
  buf[pos].a1 = a1;
  buf[pos].a2 = a2;
  buf[pos].a7 = a7;
  pos = (pos + 1) % SIZE_STR_BUF;
}

// 输出命令的调试信息
void print_etrace() {
  int i = pos;
  do {
    printf("ecall a7 [%p],argv0 [%p],argv1 [%p],argv2 [%p]", buf[i].a7,
           buf[i].a0, buf[i].a1, buf[i].a2);
    i = (i + 1) % SIZE_STR_BUF;
  } while (i != pos);
}
