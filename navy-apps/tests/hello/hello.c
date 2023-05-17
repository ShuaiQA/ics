#include <stdio.h>
#include <stdlib.h>
#include <syscall.h>
#include <unistd.h>

extern char _end;

int main(int argc, char *argv[]) {
  int i = 0;
  while (1) {
    if (i % 2000 == 0) {
      printf("hello\n");
    }
    i++;
  }
  return 0;
}
