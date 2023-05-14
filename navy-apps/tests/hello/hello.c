#include <stdio.h>
#include <stdlib.h>
#include <syscall.h>
#include <unistd.h>

int main() {
  char *m = malloc(10);
  while (1) {
    printf("%p\n", m);
  }
  return 0;
}
