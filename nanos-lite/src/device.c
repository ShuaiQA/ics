#include "klib-macros.h"
#include <common.h>
#include <stddef.h>
#include <stdio.h>

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
  strcpy(buf, keyname[ev.keycode]);
  return 1;
}

// 读取屏幕大小信息
size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  int w = io_read(AM_GPU_CONFIG).width;
  int h = io_read(AM_GPU_CONFIG).height;
  int *temp = (int *)buf;
  temp[0] = w;
  temp[1] = h;
  return 8;
}

// 把buf中的len字节写到屏幕上offset处,需要计算offset对应屏幕中的坐标
// 需要从offset中计算出x,y,w,h
size_t fb_write(const void *buf, size_t offset, size_t len) {
  // 我们规定offset的其中的低30位一次代表着x,y,w.
  // 其中h可以由len/w获得
  size_t w = offset % 1024;
  offset /= 1024;
  size_t y = offset % 1024;
  offset /= 1024;
  size_t x = offset % 1024;
  offset /= 1024;
  size_t h = len / w;
  io_write(AM_GPU_FBDRAW, x, y, buf, w, h, true);
  return len;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
