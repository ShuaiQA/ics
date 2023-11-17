#include "debug.h"
#include "klib-macros.h"
#include <common.h>

#include "amdev.h"
void init_mm(void);
void init_device(void);
void init_ramdisk(void);
void init_irq(void);
void init_fs(void);
void init_proc(void);

char buf[512];

int main() {
  extern const char logo[];
  printf("%s", logo);
  Log("'Hello World!' from Nanos-lite");
  Log("Build time: %s, %s", __TIME__, __DATE__);
  init_mm();
  init_device();
  init_ramdisk();
#ifdef HAS_CTE
  init_irq();
#endif
  init_fs();
  init_proc();
  Log("Finish initialization");

  /* io_write(AM_DISK_BLKIO, 0, buf, 0); */
  /* Log("read block no 0 %s\n", buf); */

#ifdef HAS_CTE
  yield();
#endif

  panic("Should not reach here");
}
