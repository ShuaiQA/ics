#include <stdio.h>
#include <stdlib.h>
#include <syscall.h>
#include <unistd.h>

extern char _end;

int main(int argc, char *argv[]) {
  int i = 0;
  while (1) {
    if (i % 2000 == 0) {
      printf("argc %d\n", argc);
      for (int i = 0; i < argc; i++) {
        printf("app %s\n", argv[i]);
      }
    }
  }
  return 0;
}
