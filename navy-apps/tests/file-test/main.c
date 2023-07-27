#include <assert.h>
#include <stdio.h>

int main() {
  FILE *fp = fopen("/share/files/num", "r+");
  assert(fp);

  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);
  assert(size == 5000);

  fseek(fp, 500 * 5, SEEK_SET);
  int i, n;
  // 读取文件偏移量500*5后面的字节数组以4个字节为一组,读取500次
  for (i = 500; i < 1000; i++) {
    fscanf(fp, "%d", &n);
    assert(n == i + 1);
  }

  // // 向文件的开始位置进行写数据,写入的是1001-1500
  fseek(fp, 0, SEEK_SET);
  for (i = 0; i < 500; i++) {
    fprintf(fp, "%4d\n", i + 1 + 1000);
  }

  // 再次进行判断偏移量是2500后面的数据 500-1000
  for (i = 500; i < 1000; i++) {
    fscanf(fp, "%d", &n);
    size = ftell(fp);
    printf("offset is %ld\n", size);
    printf("expect is %d fact is %d\n", i + 1, n);
    assert(n == i + 1);
  }

  fseek(fp, 0, SEEK_SET);
  for (i = 0; i < 500; i++) {
    fscanf(fp, "%d", &n);
    assert(n == i + 1 + 1000);
  }

  fclose(fp);

  printf("PASS!!!\n");

  return 0;
}
