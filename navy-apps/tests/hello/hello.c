#include <stdio.h>
#include <stdlib.h>
#include <syscall.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  char *m = malloc(10);
  printf("%p\n", m);
  // while (1) {
  //   for (int i = 0; i < argc; i++) {
  //     printf("app %s\n", argv[i]);
  //   }
  // }
  return 0;
}
