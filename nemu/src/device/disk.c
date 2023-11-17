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
  // 等待中且对于config有操作的时候才执行宿主机处理
  if (offset == config * 4 && disk_base[config] == 1) {
    if (disk_base[rw] == 0) { // 磁盘写操作
      lseek(fd, CONFIG_DISK_BLOCK_SIZE * disk_base[no], SEEK_SET);
      int len = write(fd, block, CONFIG_DISK_BLOCK_SIZE);
      Assert(len != CONFIG_DISK_BLOCK_SIZE, "写入磁盘失败");
    } else { // 读取磁盘的数据
      lseek(fd, CONFIG_DISK_BLOCK_SIZE * disk_base[no], SEEK_SET);
      int len = read(fd, block, CONFIG_DISK_BLOCK_SIZE);
      Assert(len != CONFIG_DISK_BLOCK_SIZE, "读取磁盘失败");
    }
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
  add_mmio_map("disk block", CONFIG_DISK_ADDR, block, CONFIG_DISK_BLOCK_SIZE,
               NULL);
}
