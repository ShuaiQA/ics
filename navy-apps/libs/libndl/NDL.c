#include <assert.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

// 相关的文件集合(包含设备文件)
static int evtdev = -1;   // 键盘设备
static int fbdev = -1;    // 图像设备
static int dispinfo = -1; // 屏幕信息
static int sbdev = -1;    // 音频设备
static int sbclt = -1; // 写入时是初始化音频设备,读出时是音频设备的空闲字节数
static struct timeval pre;

// 用于标记是否已经初始化了,因为当前的库给SDL和AM都需要提供接口
// 避免多次初始化中文件偏移错误
static int tag = 0;

// 记录屏幕的大小,以及画布的大小
static int screen_w = 0, screen_h = 0;
static int canvas_w, canvas_h;
static int mbv = 0; // 记录最大的音频缓冲区

uint32_t NDL_GetTicks() {
  struct timeval val;
  gettimeofday(&val, NULL);
  return (val.tv_sec - pre.tv_sec) * 1000000 + (val.tv_usec - pre.tv_usec);
}

int NDL_PollEvent(char *buf, int len) { return read(evtdev, buf, len); }

// 打开一张(*w) X (*h)的画布
// 如果*w和*h均为0, 则将系统全屏幕作为画布, 并将*w和*h分别设为系统屏幕的大小
void NDL_OpenCanvas(int *w, int *h) {
  if (*w == 0 && *h == 0) {
    *w = screen_w;
    *h = screen_h;
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

// 将pixels中存储的像素写到对应的fbdev文件的对应的偏移处
// 主要是注意像素是使用4字节表示在写入的时候需要注意4
// 是否需要判断当前的w,h超出当前的screen_w,screen_h
void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
  for (int i = 0; i < h; i++) {
    lseek(fbdev, ((y + i) * screen_w + x) * 4, SEEK_SET); // 初始的情况
    write(fbdev, pixels + w * i, w * 4);
  }
}

void NDL_OpenAudio(int freq, int channels, int samples) {
  char buf[12];
  sprintf(buf, "%d%d%d", freq, channels, samples);
  write(sbclt, buf, 12);
}

void NDL_CloseAudio() {}

int NDL_PlayAudio(void *buf, int len) { return write(sbdev, buf, len); }

int NDL_QueryAudio() {
  int free;
  read(sbclt, &free, 4);
  return mbv - free;
}

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  }
  gettimeofday(&pre, NULL);
  char buf[64];
  dispinfo = open("/proc/dispinfo", O_RDONLY);
  fbdev = open("/dev/fb", O_RDONLY);
  evtdev = open("/dev/events", O_RDONLY);
  read(dispinfo, buf, 64);
  sscanf(buf, "WIDTH: %d\nHEIGHT: %d\n", &screen_w, &screen_h);
  return 0;
}

void NDL_Quit() {
  close(dispinfo);
  close(fbdev);
  close(evtdev);
}
