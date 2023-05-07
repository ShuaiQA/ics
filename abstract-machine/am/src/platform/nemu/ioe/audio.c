#include <am.h>
#include <nemu.h>
#include <string.h>

// audio定义6个寄存器,分别用来设置freq,channels,samples
#define AUDIO_FREQ_ADDR (AUDIO_ADDR + 0x00)
#define AUDIO_CHANNELS_ADDR (AUDIO_ADDR + 0x04)
#define AUDIO_SAMPLES_ADDR (AUDIO_ADDR + 0x08)
// sbuf_size寄存器可读出流缓冲区的大小
#define AUDIO_SBUF_SIZE_ADDR (AUDIO_ADDR + 0x0c)
// init寄存器用于初始化, 将根据设置好的上面三个参数来对SDL的音频子系统进行初始化
#define AUDIO_INIT_ADDR (AUDIO_ADDR + 0x10)
// pos寄存器记录当前读取音频的位置
#define AUDIO_RPOS_ADDR (AUDIO_ADDR + 0x14)
#define BUF_SIZE 0x10000

void __am_audio_init() {}

// 设置是否需要音频
void __am_audio_config(AM_AUDIO_CONFIG_T *cfg) {
  cfg->present = true;
  cfg->bufsize = BUF_SIZE;
}

void __am_audio_ctrl(AM_AUDIO_CTRL_T *ctrl) {
  // 设置freq, channels, samples 寄存器
  outl(AUDIO_FREQ_ADDR, ctrl->freq);
  outl(AUDIO_CHANNELS_ADDR, ctrl->channels);
  outl(AUDIO_SAMPLES_ADDR, ctrl->samples);
  outl(AUDIO_INIT_ADDR, 1);
}

// 获取当前没有读取的音频的树木
void __am_audio_status(AM_AUDIO_STATUS_T *stat) {
  stat->count = inl(AUDIO_SBUF_SIZE_ADDR);
}

// 向缓冲区中写入音频数据
void __am_audio_play(AM_AUDIO_PLAY_T *ctl) {
  size_t len = ctl->buf.end - ctl->buf.start;
  while (len > BUF_SIZE - inl(AUDIO_SBUF_SIZE_ADDR)) // 空间不够等待
    ;
  uint32_t write_pos =
      inl(AUDIO_RPOS_ADDR) + inl(AUDIO_SBUF_SIZE_ADDR) % BUF_SIZE;
  if (BUF_SIZE - write_pos < len) {
    uint32_t remain = BUF_SIZE - write_pos;
    memcpy((void *)write_pos + AUDIO_SBUF_ADDR, ctl->buf.start, remain);
    memcpy((void *)AUDIO_SBUF_ADDR, ctl->buf.start + remain, len - remain);
  } else {
    memcpy((void *)AUDIO_SBUF_ADDR + write_pos, ctl->buf.start, len);
  }
  outl(AUDIO_SBUF_SIZE_ADDR, inl(AUDIO_SBUF_SIZE_ADDR) + len);
}
