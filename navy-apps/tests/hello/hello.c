#include <stdio.h>
#include <stdlib.h>
#include <syscall.h>
#include <unistd.h>

extern char _end;

int main(int argc, char *argv[]) {
  char *m = malloc(10);
  printf("malloc %p\n", m);
  printf("malloc begin is %p\n", &_end);
  // while (1) {
  //   for (int i = 0; i < argc; i++) {
  //     printf("app %s\n", argv[i]);
  //   }
  // }
  return 0;
}
