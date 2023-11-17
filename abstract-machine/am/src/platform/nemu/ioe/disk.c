#include "riscv/riscv.h"
#include <am.h>
#include <nemu.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define DISK_SIZE (DISK_ADDR + 0x00)
#define DISK_COUNT (DISK_ADDR + 0x04)
#define NO (DISK_ADDR + 0x08)
#define CNT (DISK_ADDR + 0x0c)
#define RW (DISK_ADDR + 0x10)
#define CONFIG (DISK_ADDR + 0x14)

#define DISK_BLOCK_SIZE 512

// 初始化disk是否虚拟化以及有多少块和每块的大小
void __am_disk_config(AM_DISK_CONFIG_T *cfg) {
  cfg->present = true;
  cfg->blksz = inl(DISK_SIZE);
  cfg->blkcnt = inl(DISK_COUNT);
}

// 获取当前的磁盘寄存器的状态,规定0是读写完毕,1正在读写中
void __am_disk_status(AM_DISK_STATUS_T *stat) { stat->ready = inw(CONFIG); }

/* AM_DEVREG(20, DISK_BLKIO,   WR, bool write; void *buf; int blkno); */
void __am_disk_blkio(AM_DISK_BLKIO_T *io) {
  printf("am io blkno %d\n", io->blkno);
  outl(NO, io->blkno);
  outl(RW, io->write);
  outl(CONFIG, 1);
  if (io->write) {
    memcpy((void *)DISK_BUF_ADDR, io->buf, DISK_BLOCK_SIZE);
  } else {
    memcpy(io->buf, (void *)DISK_BUF_ADDR, DISK_BLOCK_SIZE);
  }
  outl(CONFIG, 0);
}
