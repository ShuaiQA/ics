#ifndef __FS_H__
#define __FS_H__

#include <common.h>
#include <proc.h>

#ifndef SEEK_SET
enum { SEEK_SET, SEEK_CUR, SEEK_END };
#endif

// loader.c
void naive_uload(PCB *pcb, const char *filename);

// ramdisk.c
size_t get_ramdisk_size();
size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);

// fs.c
size_t fs_write(int fd, const void *buf, size_t len);

// device.c
size_t serial_write(const void *buf, size_t offset, size_t len);

#endif
