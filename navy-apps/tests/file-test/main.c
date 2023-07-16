#include <assert.h>
#include <stdio.h>

int main() {
  FILE *fp = fopen("/share/files/num", "r+");
  assert(fp);

  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);
  assert(size == 5000);

  fseek(fp, 500 * 5, SEEK_SET);
  size = ftell(fp);
  assert(size == 2500);

  int i, n;
  for (i = 500; i < 1000; i++) {
    fscanf(fp, "%d", &n);
    assert(n == i + 1);
  }
  size = ftell(fp);
  assert(size == 4999);

  fseek(fp, 0, SEEK_SET);
  for (i = 0; i < 500; i++) {
    fprintf(fp, "%4d\n", i + 1 + 1000);
  }
  size = ftell(fp);
  assert(size == 2500);

  for (i = 500; i < 1000; i++) {
    fscanf(fp, "%d", &n);
    printf("fact %d except %d\n ", n, i + 1);
    assert(n == i + 1);
  }
  size = ftell(fp);
  assert(size == 4999);

  fseek(fp, 0, SEEK_SET);
  for (i = 0; i < 500; i++) {
    fscanf(fp, "%d", &n);
    assert(n == i + 1 + 1000);
  }

  size = ftell(fp);
  assert(size == 2499);
  fclose(fp);

  printf("PASS!!!\n");

  return 0;
}
