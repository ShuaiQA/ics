#include <SDL.h>
#include <am.h>

bool ioe_init() { return true; }

// 在本质上依旧是读取或者写入某一些位置的内容然后,获取相应的结果
// 根据以往的经验app是不能够直接访问真机的具体的内容的,
// 访问需要依据系统调用来完成,在本质上依旧是文件的读写的系统调用
// 让app在系统调用的封装下依旧能够访问真机的一些功能
typedef void (*handler_t)(void *buf);

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  struct timeval now;
  gettimeofday(&now, NULL);
  uptime->us = now.tv_sec * 1000000 + now.tv_usec;
}

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  SDL_Event ev;
  if (!SDL_PollEvent(&ev)) { // 没有按键
    kbd->keycode = AM_KEY_NONE;
  } else {
    kbd->keydown = ev.type;
    kbd->keycode = ev.key.keysym.sym;
  }
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  int w = 0, h = 0;
  NDL_OpenCanvas(&w, &h);
  *cfg = (AM_GPU_CONFIG_T){
      .width = w,
      .height = h,
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  NDL_DrawRect(ctl->pixels, ctl->x, ctl->y, ctl->w, ctl->h);
}

void __am_audio_config(AM_AUDIO_CONFIG_T *cfg) {}

void __am_audio_ctrl(AM_AUDIO_CTRL_T *ctrl) {}

// 获取当前没有读取的音频的树木
void __am_audio_status(AM_AUDIO_STATUS_T *stat) {}

// 向缓冲区中写入音频数据
void __am_audio_play(AM_AUDIO_PLAY_T *ctl) {}

static void *lut[128] = {
    [AM_TIMER_UPTIME] = __am_timer_uptime,
    [AM_INPUT_KEYBRD] = __am_input_keybrd,
    [AM_GPU_CONFIG] = __am_gpu_config,
    [AM_GPU_FBDRAW] = __am_gpu_fbdraw,
    [AM_AUDIO_CONFIG] = __am_audio_config,
    [AM_AUDIO_CTRL] = __am_audio_ctrl,
    [AM_AUDIO_STATUS] = __am_audio_status,
    [AM_AUDIO_PLAY] = __am_audio_play,
};

void ioe_read(int reg, void *buf) { ((handler_t)lut[reg])(buf); }
void ioe_write(int reg, void *buf) { ((handler_t)lut[reg])(buf); }
