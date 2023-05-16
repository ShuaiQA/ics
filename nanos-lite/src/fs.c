#include "am.h"
#include "debug.h"
#include <fs.h>

typedef size_t (*ReadFn)(void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn)(const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
  size_t open_offset;
} Finfo;

enum { FD_STDIN, FD_STDOUT, FD_STDERR, FD_EVENT, FD_FB, FD_DISP };

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

#define NR_REGEX ARRLEN(file_table)

// 由于当前的文件的大小是固定的,在读写偏移的时候不能够超过文件的大小,防止修改别的文件
static Finfo file_table[] __attribute__((used)) = {
    [FD_STDIN] = {"stdin", 0, 0, invalid_read, invalid_write},
    [FD_STDOUT] = {"stdout", 0, 0, invalid_read, serial_write},
    [FD_STDERR] = {"stderr", 0, 0, invalid_read, serial_write},
    [FD_EVENT] = {"/dev/events", 0, 0, events_read, invalid_write},
    [FD_FB] = {"/dev/fb", 0, 0, invalid_read, fb_write},
    [FD_DISP] = {"/proc/dispinfo", 64, 0, dispinfo_read, NULL},

#include "files.h"
};

// 根据pathname找到对应的文件描述符
int fs_open(const char *pathname, int flags, int mode) {
  for (int i = 0; i < NR_REGEX; i++) {
    if (strcmp(pathname, file_table[i].name) == 0) {
      file_table[i].open_offset = 0;
      return i;
    }
  }
  panic("没有找到当前文件");
  return -1;
}

// 从下标fd中读取相关的字符len的长度,读取到buf中
size_t fs_read(int fd, void *buf, size_t len) {
  if (file_table[fd].read != NULL) {
    return file_table[fd].read(buf, file_table[fd].open_offset, len);
  }
  size_t offset = file_table[fd].disk_offset;
  size_t size = file_table[fd].size;
  size_t open_offset = file_table[fd].open_offset;
  size_t remain_size = size - open_offset;
  size_t result_size = remain_size > len ? len : remain_size;
  file_table[fd].open_offset += result_size;
  return ramdisk_read(buf, offset + open_offset, result_size);
}

// 按照手册的说法即使是offset跳过了超出了文件的长度也应该写入(但是这里限制了文件的长度)
size_t fs_write(int fd, const void *buf, size_t len) {
  if (file_table[fd].write != NULL) {
    return file_table[fd].write(buf, file_table[fd].open_offset, len);
  }
  size_t remain_size = file_table[fd].size - file_table[fd].open_offset;
  remain_size = remain_size > len ? len : remain_size;
  file_table[fd].open_offset += remain_size;
  return ramdisk_write(buf,
                       file_table[fd].disk_offset + file_table[fd].open_offset,
                       remain_size);
}

size_t fs_lseek(int fd, size_t offset, int whence) {
  switch (whence) {
  case SEEK_SET:
    file_table[fd].open_offset = offset;
    break;
  case SEEK_CUR:
    file_table[fd].open_offset += offset;
    break;
  case SEEK_END:
    file_table[fd].open_offset = file_table[fd].size + offset;
    break;
  default:
    panic("fs_lseek is failed whence is %d", whence);
  }
  // 不能够超出文件的大小
  return file_table[fd].open_offset > file_table[fd].size
             ? file_table[fd].size
             : file_table[fd].open_offset;
}

int fs_close(int fd) { return 0; }

size_t getfilesize(int fd) { return file_table[fd].size; }

// 初始化主要是对文件相关的读写指针进行初始化,比如说FD_STDIN,FD_STDOUT,FD_STDERR文件
// 和其余的文件的读写是不一样的,我们需要对相应的确定的文件描述符指定确定的读写函数
// 对文件记录表中/dev/fb的大小进行初始化
void init_fs() {
  // TODO: initialize the size of /dev/fb
}
