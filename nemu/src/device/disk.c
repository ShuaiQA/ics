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
 *NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A P
 *
 ** See the Mulan PSL v2 for more details
 ****************************************************************************************/

#include <assert.h>
#include <debug.h>
#include <device/map.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

enum {
  block_size,
  block_count,
  no,
  cnt,
  rw,
  config,
};

// 根据传入的字符串进行文件查找
int fd;
uint32_t *disk_base;
uint8_t *block;

static void disk_io_handler(uint32_t offset, int len, bool is_write) {
  switch (offset) {
  /* We bind the serial port with the host stderr in NEMU. */
  case config * 4:
    if (is_write) {
    }
  default:
    panic("do not support offset = %d", offset);
  }
}

// 初始化磁盘块大小和多少块
void init_disk() {
  int len = lseek(fd, 0, SEEK_END);
  int bs = len / CONFIG_DISK_BLOCK_SIZE;
  uint32_t space_size = sizeof(uint32_t) * config;
  disk_base = (uint32_t *)new_space(space_size);
  disk_base[block_size] = CONFIG_DISK_BLOCK_SIZE;
  disk_base[block_count] = bs;
  add_mmio_map("disk", CONFIG_DISK_CTL_MMIO, disk_base, space_size,
               disk_io_handler);

  block = new_space(CONFIG_DISK_BLOCK_SIZE);
  add_mmio_map("disk block", CONFIG_DISK_ADDR, block, CONFIG_DISK_BLOCK_SIZE, NULL);
}
