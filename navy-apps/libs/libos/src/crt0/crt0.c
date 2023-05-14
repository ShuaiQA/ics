#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[], char *envp[]);
extern char **environ;

// 当前的args也就是相当于a0
void call_main(uintptr_t *args) {
  char *buf = (char *)args;
  int pos = *(int *)(buf);
  int argc = *(int *)(buf + 4);
  printf("use pos is %d argc %d\n", pos, argc);
  char *ags = (char *)args + pos;
  char *argv[argc + 1];
  for (int i = 0; i < argc; i++) {
    argv[i] = ags;
    size_t s = strlen(ags);
    ags += s;
  }
  for (int i = 0; i < argc; i++) {
    printf("use argv %s\n", argv[i]);
  }
  // 对字符串buf进行拆分获取argc,argv
  char *empty[] = {NULL};
  environ = empty;
  exit(main(argc, argv, empty));
  assert(0);
}
