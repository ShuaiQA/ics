#include <BMP.h>
#include <NDL.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
  NDL_Init(0);
  int w, h;

  NDL_LoadWH(&w, &h);
  printf("w is %d h is %d \n", w, h);
  void *bmp = BMP_Load("/share/pictures/projectn.bmp", &w, &h);
  assert(bmp);
  NDL_OpenCanvas(&w, &h);
  NDL_DrawRect(bmp, 0, 0, w, h);
  free(bmp);
  NDL_Quit();
  printf("Test ends! Spinning...\n");
  while (1)
    ;
  return 0;
}
