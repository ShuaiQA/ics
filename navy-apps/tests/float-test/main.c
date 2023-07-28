#include <fixedptc.h>
#include <stdio.h>

int main() {
  fixedpt a = fixedpt_rconst(2.5);
  int d = 2;
  fixedpt b = fixedpt_fromint(3);
  fixedpt c = fixedpt_mul(a, b);
  fixedpt f = fixedpt_divi(a, d);
  char buf[30];
  fixedpt_str(c, buf, -2);
  printf("%s\n", buf);
  fixedpt_str(f, buf, -2);
  printf("%s\n", buf);

  f = fixedpt_div(a, b);
  fixedpt_str(f, buf, -2);
  printf("%s\n", buf);
  return 0;
}
