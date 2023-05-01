#ifndef __FS_H__
#define __FS_H__

#include <common.h>

/* Seek from beginning of file.  */
/* Seek from current position.  */
/* Seek from end of file.  */
#ifndef SEEK_SET
enum { SEEK_SET, SEEK_CUR, SEEK_END };
#endif
#define ARRLEN(arr) (int)(sizeof(arr) / sizeof(arr[0]))

size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);

size_t fs_write(int fd, const void *buf, size_t len);
size_t fs_read(int fd, void *buf, size_t len);
int fs_close(int fd);
size_t fs_lseek(int fd, size_t offset, int whence);
int fs_open(const char *pathname, int flags, int mode);

size_t serial_write(const void *buf, size_t offset, size_t len);
size_t events_read(void *buf, size_t offset, size_t len);

#endif
