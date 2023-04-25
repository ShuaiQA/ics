#include "debug.h"
#include "sdb.h"
#include <assert.h>
#include <elf.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct Elf { // 记录函数的起始位置、函数的大小、名字
  uint32_t st_value;
  uint32_t st_size;
  char name[30];
} funcs;

typedef struct funcbuf { // 标记函数的起始位置
  uint32_t val;
  char name[30];
  int pos;
} funtrace;

static funcs buf[10000]; // 记录当前有多少个函数
static size_t num = 0;
static funtrace ff[20]; // 环形缓冲区记录函数调用过程
static size_t cnt = 0;

void print_fun_buf() {
  int i = cnt;
  do {
    printf("%08x\t%s\n", ff[i].val, ff[i].name);
    i = (i + 1) % 20;
  } while (i != cnt);
}

// 根据传入的pc的值,根据pc值来确定函数的路径并添加到ff数组中
void new_fun(uint32_t pc) {
  int pos = -1;
  for (int i = 0; i < num; i++) {
    if (pc >= buf[i].st_value && pc <= buf[i].st_value + buf[i].st_size) {
      pos = i;
    }
  }
  assert(pos != -1);

  // 获取前一个pos的对应位置,查看当前是否对应
  funtrace pre = ff[(cnt + 19) % 20];
  if (pc < buf[pre.pos].st_value ||
      pc > buf[pre.pos].st_value + buf[pre.pos].st_size) {
    ff[cnt].val = pc;
    strcpy(ff[cnt].name, buf[pos].name);
    ff[cnt].pos = pos;
    cnt = (cnt + 1) % 20;
  }
}

// 该函数主要是为了解析elf文件然后将获取的函数的起始地址、大小、函数名
// 放到函数缓冲区中,为了以后执行命令的时候进行函数的判断
void load_elf(char *elf_file) {
  if (elf_file == NULL) {
    printf("no elf file\n");
    return;
  }
  int fd;
  char *file_mmbase;
  struct stat file_status;
  size_t fsize;
  Elf32_Ehdr *ehdr;
  Elf32_Shdr *shdrs;
  size_t shnum;
  // 打开目标文件标识符
  fd = open(elf_file, O_RDONLY);
  // 获取文件标识符相关的文件信息
  fstat(fd, &file_status);
  fsize = (size_t)file_status.st_size;
  // 从fd文件中获取相应的fsize大小,将返回的地址使用file_mmbase中,PROT_READ映射区域可被读取
  file_mmbase = mmap(NULL, fsize, PROT_READ, MAP_PRIVATE, fd, (off_t)0);
  // 将相关的字符流转换成头文件
  ehdr = (Elf32_Ehdr *)file_mmbase;
  // Section header 起始位置
  shdrs = (Elf32_Shdr *)(file_mmbase + ehdr->e_shoff);
  shnum = ehdr->e_shnum; // 获取Section header的个数
  for (size_t i = 0; i < shnum; i++) {
    // 获取每一个Section header
    Elf32_Shdr *shdr = &shdrs[i];
    if (shdr->sh_type == SHT_SYMTAB) { // 如果当前的类型是symtab
      // 获取当前的.symtab的偏移位置
      Elf32_Sym *syms = (Elf32_Sym *)(file_mmbase + shdr->sh_offset);
      // 获取当前.symtab的大小然后除以每一个的大小,获取有多少个.symtab
      size_t entries = shdr->sh_size / shdr->sh_entsize;
      // 获取symtab的name
      const char *strtab = file_mmbase + shdrs[shdr->sh_link].sh_offset;
      for (size_t i = 0; i < entries; i++) {
        Elf32_Sym *sym = &syms[i];
        if (ELF32_ST_TYPE(sym->st_info) == STT_FUNC) {
          buf[num].st_value = sym->st_value;
          buf[num].st_size = sym->st_size;
          strcpy(buf[num].name, strtab + sym->st_name);
          if (strcmp(strtab + sym->st_name, "_start") == 0) {
            buf[num].st_size = 0xc;
          }
          num++;
          if (num == 10000) {
            panic("函数缓冲区不足");
          }
        }
      }
    }
  }
  // 查看当前的函数集合
  // printf("num is %zu\n", num);
  // for (int i = 0; i < num; i++) {
  //   printf("%08x\t%d\t%s\n", buf[i].st_value, buf[i].st_size, buf[i].name);
  // }
  (void)munmap(file_mmbase, fsize);
  (void)close(fd);
}
