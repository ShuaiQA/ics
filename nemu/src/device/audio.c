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
#include <common.h>
#include <device/map.h>
#include <string.h>

enum {
  reg_freq,
  reg_channels,
  reg_samples,
  reg_sbuf_size,
  reg_init,
  reg_count,
  nr_reg
};

static uint32_t *audio_base = NULL; // 寄存器
static uint8_t *sbuf = NULL;        // 缓冲区

static Uint8 *audio_pos;

void audio_callback(void *userdata, Uint8 *stream, int len) {
  if (audio_base[reg_count] >= audio_base[reg_sbuf_size]) {
    return;
  }
  if (audio_base[reg_count] + len > audio_base[reg_sbuf_size])
    len = audio_base[reg_sbuf_size] - audio_base[reg_count];
  memcpy(stream, audio_pos, len);
  audio_pos += len;
  audio_base[reg_count] += len;
}

// 如果偏移量在前面的三个寄存器需要设置相应的初始化
static void audio_io_handler(uint32_t offset, int len, bool is_write) {
  SDL_AudioSpec loaded_wav_spec;
  switch (offset) {
  case reg_freq * 4:
    loaded_wav_spec.freq = audio_base[reg_freq];
    break;
  case reg_channels * 4:
    loaded_wav_spec.channels = audio_base[reg_channels];
    break;
  case reg_samples * 4:
    loaded_wav_spec.samples = audio_base[reg_samples];
    break;
  default:
    panic("do not support offset = %d", offset);
  }
  audio_pos = sbuf;
  audio_base[reg_count] = 0;
  loaded_wav_spec.callback = audio_callback;
  loaded_wav_spec.userdata = sbuf;
  int iscapture = 0;
  int allowed_changes = 0;

  const char *device_name = SDL_GetAudioDeviceName(0, iscapture);
  SDL_AudioDeviceID device = SDL_OpenAudioDevice(
      device_name, iscapture, &loaded_wav_spec, NULL, allowed_changes);
  SDL_PauseAudioDevice(device, 0);
  while (audio_base[reg_count] < audio_base[reg_sbuf_size]) {
    SDL_Delay(100);
  }
  SDL_CloseAudioDevice(device);
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
