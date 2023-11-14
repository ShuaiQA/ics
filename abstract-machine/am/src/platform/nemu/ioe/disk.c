#include <am.h>
#include <nemu.h>

// 初始化disk是否虚拟化以及有多少块和每块的大小
void __am_disk_config(AM_DISK_CONFIG_T *cfg) { cfg->present = false; }

void __am_disk_status(AM_DISK_STATUS_T *stat) {}

void __am_disk_blkio(AM_DISK_BLKIO_T *io) {}
