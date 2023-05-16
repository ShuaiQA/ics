#include <am.h>
#include <nemu.h>
#include <stdint.h>
#include <string.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

static uint32_t width, heigh;

void __am_gpu_init() {
  uint32_t f1 = inl(VGACTL_ADDR);
  heigh = f1 & 0xffff;
  width = f1 >> 16;
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  *cfg = (AM_GPU_CONFIG_T){
      .width = width,
      .height = heigh,
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  uint32_t *src = (uint32_t *)ctl->pixels; // FB_ADDR
  uint32_t *dst = (uint32_t *)FB_ADDR;
  uint32_t f1 = inl(VGACTL_ADDR);
  uint32_t heigh = f1 & 0xffff;
  uint32_t width = f1 >> 16;
  int cnt = 0;
  for (int j = ctl->y; j < ctl->y + ctl->h; j++) {
    for (int i = ctl->x; i < ctl->x + ctl->w; i++) {
      fb[i + j * 400] = p[cnt++];
    }
  }
  if (ctl->sync) {
    outl(SYNC_ADDR, 1); // 直接根据对应的内存地址对nemu的[1]下标进行修改
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) { status->ready = true; }
