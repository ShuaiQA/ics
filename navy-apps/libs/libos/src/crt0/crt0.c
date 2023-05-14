#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[], char *envp[]);
extern char **environ;

// 当前的args也就是相当于a0
void call_main(uintptr_t *args) {
  int argc = *(int *)(args);
  printf("use argc %d\n", argc);
  char *ags = (char *)args + 4;
  char *argv[argc + 1];
  for (int i = argc - 1; i >= 0; i--) {
    argv[i] = ags;
    ags += strlen(ags) + 1;
  }
  argv[argc] = NULL;
  for (int i = 0; i < argc; i++) {
    printf("use argv %s\n", argv[i]);
  }
  char *empty[] = {NULL};
  environ = empty;
  exit(main(argc, argv, empty));
  assert(0);
}
