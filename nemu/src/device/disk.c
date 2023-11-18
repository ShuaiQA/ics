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
  rw,
  config,
  bl_reg,
};

// 根据传入的字符串进行文件查找
int fd;
uint32_t *disk_base;
uint8_t *block;

static void disk_io_handler(uint32_t offset, int len, bool is_write) {
  // 等待中且对于config有操作的时候才执行宿主机处理
  if (offset == config * 4 && disk_base[config] == 1) {
    if (disk_base[rw] == 0) { // 磁盘读操作
      Log("duqucaozuo ");
      lseek(fd, CONFIG_DISK_BLOCK_SIZE * disk_base[no], SEEK_SET);
      int len = read(fd, block, CONFIG_DISK_BLOCK_SIZE);
      printf("len is %d", len);
      for (int i = 0; i < len; i++) {
        printf("%c", block[i]);
      }
      Assert(len != CONFIG_DISK_BLOCK_SIZE, "读取磁盘失败");
    } else {
      lseek(fd, CONFIG_DISK_BLOCK_SIZE * disk_base[no], SEEK_SET);
      int len = write(fd, block, CONFIG_DISK_BLOCK_SIZE);
      Assert(len != CONFIG_DISK_BLOCK_SIZE, "写入磁盘失败");
    }
  }
  if (offset == no * 4) {
    Log("no is %d", disk_base[no]);
  } else if (offset == rw * 4) {
    Log("rw is %d", disk_base[rw]);
  } else if (offset == config * 4) {
    Log("config is %d", disk_base[config]);
  }
}

// 初始化磁盘块大小和多少块
void init_disk() {
  int len = lseek(fd, 0, SEEK_END);
  int bs = len / CONFIG_DISK_BLOCK_SIZE;
  uint32_t space_size = sizeof(uint32_t) * bl_reg;
  disk_base = (uint32_t *)new_space(space_size);
  disk_base[block_size] = CONFIG_DISK_BLOCK_SIZE;
  disk_base[block_count] = bs;
  add_mmio_map("disk", CONFIG_DISK_CTL_MMIO, disk_base, space_size,
               disk_io_handler);

  block = new_space(CONFIG_DISK_BLOCK_SIZE);
  add_mmio_map("disk block", CONFIG_DISK_ADDR, block, CONFIG_DISK_BLOCK_SIZE,
               NULL);
}

// 此处使用LOG出现了段错误？
// 问题是当前的日记文件并没有进行初始化
void set_disk_file(char *disk) {
  char buf[512];
  fd = open(disk, O_RDWR);
  Assert(fd >= 3, "open fail fd is %d", fd);
  printf("disk");
  int len = read(fd, buf, 512);
  for (int i = 0; i < len; i++) {
    printf("%c", buf[i]);
  }
  lseek(fd, 0, SEEK_SET);
}
