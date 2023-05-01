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

enum { FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB };

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

#define ARRLEN(arr) (int)(sizeof(arr) / sizeof(arr[0]))
#define NR_REGEX ARRLEN(file_table)

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
    [FD_STDIN] = {"stdin", 0, 0, invalid_read, invalid_write},
    [FD_STDOUT] = {"stdout", 0, 0, invalid_read, invalid_write},
    [FD_STDERR] = {"stderr", 0, 0, invalid_read, invalid_write},
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
  assert(0); // 没有找到当前的文件
  return -1;
}

// 从下标fd中读取相关的字符len的长度,读取到buf中
size_t fs_read(int fd, void *buf, size_t len) {
  size_t offset = file_table[fd].disk_offset;
  size_t size = file_table[fd].size;
  size_t open_offset = file_table[fd].open_offset;
  if (open_offset <= size) { // 文件的偏移量需要小于文件的大小才可以读取
    if (open_offset + len < size) { // 文件的偏移量加上读取的数字小于size
      ramdisk_read(buf, offset + open_offset, len);
      return len;
    } else { // 文件的偏移量加上长度大于size
      int read_cnt = size - open_offset;
      ramdisk_read(buf, offset + open_offset, read_cnt);
      return read_cnt;
    }
  }
  // 偏移量大于等于size返回-1
  return -1;
}

// 按照手册的说法即使是offset跳过了超出了文件的长度也应该写入
size_t fs_write(int fd, const void *buf, size_t len) {
  return ramdisk_write(
      buf, file_table[fd].disk_offset + file_table[fd].open_offset, len);
}

size_t fs_lseek(int fd, size_t offset, int whence) {
  if (whence == SEEK_SET) {
    file_table[fd].open_offset = offset;
  } else if (whence == SEEK_CUR) {
    file_table[fd].open_offset += offset;
  } else {
    file_table[fd].open_offset = file_table[fd].size + offset;
  }
  return file_table[fd].open_offset;
}

int fs_close(int fd) { return 0; }

void init_fs() {
  // TODO: initialize the size of /dev/fb
}
