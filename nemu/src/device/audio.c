/***************************************************************************************
 * Copyright (c) 2014-2022 Zihao Yu, Nanjing University
 *
 * NEMU is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan
 *PSL v2. You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 *KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 *NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 ***************************************************************************************/

#include <SDL2/SDL.h>
#include <assert.h>
#include <common.h>
#include <device/map.h>
#include <string.h>

enum {
  reg_freq,
  reg_channels,
  reg_samples,
  reg_sbuf_size, // 有用音频的长度
  reg_init,
  reg_rpos, // 下一个读取的位置下标(环形缓冲区)
  nr_reg
};

static uint32_t *audio_base = NULL; // 寄存器
static uint8_t *sbuf = NULL;        // 缓冲区

SDL_AudioSpec loaded_wav_spec;

// 根据环形缓冲区读取数据
void audio_callback(void *userdata, Uint8 *stream, int len) {
  if (audio_base[reg_sbuf_size] <= 0) // 当前没有数据
    return;
  // 防止读取的长度大于总的长度,设置len
  if (audio_base[reg_sbuf_size] < len) // 防止读取的长度超过当前数据
    len = audio_base[reg_sbuf_size];
  // 拷贝一定len字节到stream中
  int remain = CONFIG_SB_SIZE - audio_base[reg_rpos];
  if (remain < len) {
    memcpy(stream, sbuf + audio_base[reg_rpos], remain);
    memcpy(stream + remain, sbuf, len - remain);
    audio_base[reg_rpos] = len - remain;
  } else {
    memcpy(stream, sbuf + audio_base[reg_rpos], len);
    audio_base[reg_rpos] += len;
  }
  audio_base[reg_sbuf_size] -= len;
}

// 如果偏移量在前面的三个寄存器需要设置相应的初始化
static void audio_io_handler(uint32_t offset, int len, bool is_write) {
  if (offset == 0x10 && is_write) { // 进行初始化
    loaded_wav_spec.freq = audio_base[reg_freq];
    loaded_wav_spec.channels = audio_base[reg_channels];
    loaded_wav_spec.samples = audio_base[reg_samples];
    loaded_wav_spec.callback = audio_callback;
    loaded_wav_spec.userdata = sbuf;
    SDL_OpenAudio(&loaded_wav_spec, NULL);
  } else {
    SDL_PauseAudio(0);
    while (audio_base[reg_sbuf_size] > 0)
      ;
  }
}

void init_audio() {
  uint32_t space_size = sizeof(uint32_t) * nr_reg;
  audio_base = (uint32_t *)new_space(space_size);
#ifdef CONFIG_HAS_PORT_IO
  add_pio_map("audio", CONFIG_AUDIO_CTL_PORT, audio_base, space_size,
              audio_io_handler);
#else
  add_mmio_map("audio", CONFIG_AUDIO_CTL_MMIO, audio_base, space_size,
               audio_io_handler);
#endif

  sbuf = (uint8_t *)new_space(CONFIG_SB_SIZE);
  add_mmio_map("audio-sbuf", CONFIG_SB_ADDR, sbuf, CONFIG_SB_SIZE, NULL);
}
