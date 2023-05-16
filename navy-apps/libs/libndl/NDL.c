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

// 相关的文件集合(包含设备文件)
static int evtdev = -1;  // events file
static int fbdev = -1;   // gpu file
static int sb = -1;      // audio file
static int dispdev = -1; // gpu config file
static int sbctl = -1;   // audio config file

// 用于标记是否已经初始化了,因为当前的库给SDL和AM都需要提供接口
// 避免多次初始化中文件偏移错误
static int tag = 0;

// 记录屏幕的大小,以及画布的大小
static int screen_w = 0, screen_h = 0;
static int canvas_w, canvas_h;

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

int NDL_PollEvent(char *buf, int len) { return read(evtdev, buf, len); }

// 打开一张(*w) X (*h)的画布
// 如果*w和*h均为0, 则将系统全屏幕作为画布, 并将*w和*h分别设为系统屏幕的大小
void NDL_OpenCanvas(int *w, int *h) {
  assert(*w <= screen_w && *h <= screen_h); // 打开画布的大小应该小于屏幕的大小
  if (*w == 0 && *h == 0) {
    char buf[64];
    read(dispdev, buf, 64);
    sscanf(buf, "WIDTH: %d\nHEIGHT: %d\n", &canvas_w, &canvas_h);
    *w = canvas_w;
    *h = canvas_h;
  } else {
    canvas_w = *w;
    canvas_h = *h;
  }
  if (getenv("NWM_APP")) { // 当前没有这个环境变量
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

// 将pixels中存储的像素写到对应的fbdev文件的对应的偏移处
// 主要是注意像素是使用4字节表示在写入的时候需要注意4
// 是否需要判断当前的w,h超出当前的screen_w,screen_h
void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
  assert(w <= screen_w || h <= screen_h);
  for (int i = 0; i < h; i++) {
    lseek(fbdev, ((y + i) * screen_w + x) * 4, SEEK_SET); // 初始的情况
    write(fbdev, pixels + w * i, w * 4);
  }
}

void NDL_OpenAudio(int freq, int channels, int samples) {
  int buf[3];
  buf[0] = freq;
  buf[1] = channels;
  buf[2] = samples;
  write(sbctl, buf, 12);
}

void NDL_CloseAudio() {}

int NDL_PlayAudio(void *buf, int len) { return write(sb, buf, len); }

int NDL_QueryAudio() {
  int *buf;
  read(sbctl, buf, 4);
  return buf[0];
}

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  }
  if (tag == 0) {
    char buf[64];
    dispdev = open("/proc/dispinfo", O_RDONLY);
    fbdev = open("/dev/fb", O_RDONLY);
    sb = open("/dev/sb", O_RDWR);
    sbctl = open("/dev/sbctl", O_RDWR);
    evtdev = open("/dev/events", O_RDONLY);
    read(dispdev, buf, 64);
    sscanf(buf, "WIDTH: %d\nHEIGHT: %d\n", &screen_w, &screen_h);
    close(dispdev);
    // printf("screen_w %d screen_h %d\n", screen_w, screen_h);
    tag = 1;
  }
  return 0;
}

void NDL_Quit() {
  if (tag) {
    close(fbdev);
    close(evtdev);
    close(sb);
    close(sbctl);
    tag = 0;
  }
}
