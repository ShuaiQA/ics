#include <common.h>
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

size_t dispinfo_read(void *buf, size_t offset, size_t len) { return 0; }

size_t fb_write(const void *buf, size_t offset, size_t len) { return 0; }

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
