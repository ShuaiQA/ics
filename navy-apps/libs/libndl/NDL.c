#include <assert.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;

// 以毫秒为单位返回系统时间
uint32_t NDL_GetTicks() {
  struct timeval val;
  int ret = gettimeofday(&val, NULL);
  if (ret == -1) {
    assert(0);
    return -1;
  }
  return val.tv_sec * 1000000 + val.tv_usec;
}

int NDL_PollEvent(char *buf, int len) {
  int fd = open("/dev/events", O_RDONLY);
  assert(fd);
  return read(fd, buf, 15);
}

void NDL_LoadWH(int *w, int *h) {
  FILE *fd = fopen("/proc/dispinfo", "r");
  fscanf(fd, "width:%d\nheight:%d\n", w, h);
}

// 打开一张(*w) X (*h)的画布
// 如果*w和*h均为0, 则将系统全屏幕作为画布, 并将*w和*h分别设为系统屏幕的大小
void NDL_OpenCanvas(int *w, int *h) {
  if (*w == 0 && *h == 0) {
    *w = 400;
    *h = 300;
  }
  if (getenv("NWM_APP")) {
    int fbctl = 4;
    fbdev = 5;
    screen_w = *w;
    screen_h = *h;
    char buf[64];
    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    // let NWM resize the window and create the frame buffer
    write(fbctl, buf, len);
    while (1) {
      // 3 = evtdev
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0)
        continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0)
        break;
    }
    close(fbctl);
  }
}

// 主要是注意像素是使用4字节表示在写入的时候需要注意4
void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
  int fd = open("/dev/fb", O_RDWR);
  int sw = 400, sh = 300;
  printf("%d %d %d %d\n", x, y, w, h);
  printf("NDL_DrawRect\n");
  lseek(fd, (y * sw + x) * 4, SEEK_SET); // 初始的情况
  for (int i = 0; i < h; i++) {
    lseek(fd, ((y + i) * sw + x) * 4, SEEK_SET); // 初始的情况
    write(fd, pixels + w * i, w * 4);
  }
  close(fd);
}

void NDL_OpenAudio(int freq, int channels, int samples) {}

void NDL_CloseAudio() {}

int NDL_PlayAudio(void *buf, int len) { return 0; }

int NDL_QueryAudio() { return 0; }

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  }
  return 0;
}

void NDL_Quit() {}
