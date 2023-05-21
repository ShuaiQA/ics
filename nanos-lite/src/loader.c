#include "am.h"
#include "debug.h"
#include "memory.h"
#include <elf.h>
#include <fs.h>
#include <proc.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#ifdef __LP64__
#define Elf_Ehdr Elf64_Ehdr
#define Elf_Phdr Elf64_Phdr
#else
#define Elf_Ehdr Elf32_Ehdr
#define Elf_Phdr Elf32_Phdr
#endif

// 解析elf文件流,将type==LOAD需要加载的部分加载到相应的页表中,并建立起虚拟地址空间的映射关系
uintptr_t load_segement(PCB *pcb, char *date) {
  Elf_Ehdr *hd = (Elf_Ehdr *)date;
  assert(*(uint32_t *)hd->e_ident == 0x464c457f);
  uintptr_t entry = hd->e_entry;
  // printf("entry is %x num is %d offset is %x\n", entry, hd->e_phnum,
  //        hd->e_phoff);
  Elf_Phdr *phs = (Elf_Phdr *)(date + hd->e_phoff);
  for (int i = 0; i < hd->e_phnum; i++) {
    Elf_Phdr ph = phs[i];
    if (ph.p_type == PT_LOAD) {
      Log("%x %x %x %x", ph.p_vaddr, ph.p_memsz, ph.p_offset, ph.p_filesz);
      // 将当前的虚拟地址加载到相关的物理页面中,获取虚拟地址,虚拟地址大小.
      // 加载数据的位置(文件偏移量),加载数据的大小.
      uintptr_t vaddr = ph.p_vaddr, vsize = ph.p_memsz, f_offset = ph.p_offset,
                f_size = ph.p_filesz;
      uintptr_t size = 0;          // 记录了当前移动了多少的数据
      char *pos = date + f_offset; // 记录了移动到哪里了
      while (size + PGSIZE <= f_size) { // 设置p_filesz
        char *page = new_page(1);
        memmove(page, pos, PGSIZE);
        map(&pcb->as, (char *)vaddr, page, 0);
        Log("f_size vaddr %x to paddr %x", vaddr, page);
        pos += PGSIZE;
        vaddr += PGSIZE; // 虚拟地址自增为了映射
        size += PGSIZE;
      }
      // 设置一个临界的页,也就是f_size剩下的和vsize开始的(如果有的话)
      if (f_size - size != 0) {
        char *page = new_page(1);
        uintptr_t len = f_size - size;
        // 注意只是移动了len!=4096个,默认new_page的字符是0
        memmove(page, pos, len);
        map(&pcb->as, (char *)vaddr, page, 0);
        Log("f_size and vsize %x to paddr %x", vaddr, page);
        pos += PGSIZE;
        size += PGSIZE;
        vaddr += PGSIZE;
      }
      while (size <= vsize) {
        char *page = new_page(1); // 默认是0
        map(&pcb->as, (char *)vaddr, page, 0);
        Log("vsize vaddr %x to paddr %x", vaddr, page);
        pos += PGSIZE;
        vaddr += PGSIZE; // 虚拟地址自增为了映射
        size += PGSIZE;
      }
    }
  }
  return entry;
}

// 根据文件的名字
static uintptr_t loader(PCB *pcb, const char *filename) {
  int fd = fs_open(filename, 0, 0);
  size_t size = getfilesize(fd);
  fs_lseek(fd, 0, SEEK_SET);
  char *date = malloc(size);
  fs_read(fd, date, size);
  uintptr_t entry = load_segement(pcb, date);
  fs_close(fd);
  free(date);
  return entry;
}

// 加载一个filename的用户程序,返回该用户程序的最初执行的入口
void *naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  return ((void (*)())entry);
}
