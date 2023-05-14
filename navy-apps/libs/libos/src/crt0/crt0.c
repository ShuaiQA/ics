#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[], char *envp[]);
extern char **environ;

// 当前的args也就是相当于a0
void call_main(uintptr_t *args) {
  char *buf = (char *)args;
  // 对字符串buf进行拆分获取argc,argv
  char *empty[] = {NULL};
  environ = empty;
  exit(main(0, empty, empty));
  assert(0);
}
