#include "amdev.h"
#include "klib-macros.h"
#include <common.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
#define MULTIPROGRAM_YIELD() yield()
#else
#define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) [AM_KEY_##key] = #key,

static const char *keyname[256]
    __attribute__((used)) = {[AM_KEY_NONE] = "NONE", AM_KEYS(NAME)};

// 串口写入操作
size_t serial_write(const void *buf, size_t offset, size_t len) {
  char *temp = (char *)buf;
  for (size_t i = 0; i < len; i++) {
    putch(temp[i]);
  }
  return len;
}

// 根据keycode获取keyname,按下一次包含输出2次(按下和弹起)
size_t events_read(void *buf, size_t offset, size_t len) {
  AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
  if (ev.keycode == AM_KEY_NONE) {
    return 0;
  }
  if (ev.keydown == 1) {
    sprintf(buf, "%s ", "kd");
  } else {
    sprintf(buf, "%s ", "ku");
  }
  sprintf(buf, "%s\n", keyname[ev.keycode]);
  printf("%d %s \n", strlen(buf), (char *)buf);
  return strlen(buf);
}

// 读取屏幕大小信息,根据一定的格式向buf中写入数据
size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  int w = io_read(AM_GPU_CONFIG).width;
  int h = io_read(AM_GPU_CONFIG).height;
  int c = sprintf(buf, "width:%d\nheight:%d\n", w, h);
  return c;
}

size_t fb_write(void *buf, size_t offset, size_t len) {
  int x = offset / 4 % 400;
  int y = offset / 4 / 400;
  io_write(AM_GPU_FBDRAW, x, y, buf, len / 4, 1, true);
  return len;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
