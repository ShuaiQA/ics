#include <stdio.h>
#include <unistd.h>

int main() {
  write(1, "Hello World!\n", 13);
  int i = 2;
  while(1){
    if (i % 10 == 0) {
      printf("Hello World from Navy-apps for the %dth time!\n", i++);
    }
    i++;
  }
  _exit(0);
}
