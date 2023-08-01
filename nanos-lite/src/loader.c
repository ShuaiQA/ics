#include "common.h"
#include "debug.h"
#include <elf.h>
#include <fs.h>
#include <proc.h>
#include <stdint.h>

#ifdef __LP64__
#define Elf_Ehdr Elf64_Ehdr
#define Elf_Phdr Elf64_Phdr
#else
#define Elf_Ehdr Elf32_Ehdr
#define Elf_Phdr Elf32_Phdr
#endif

extern uint8_t ramdisk_start;

// 根据字符数组获取需要加载到内容中
uintptr_t load_segement(PCB *pcb, char *date) {
  Elf_Ehdr *hd = (Elf_Ehdr *)date;
  assert(*(uint32_t *)hd->e_ident == 0x464c457f);
  uintptr_t entry = hd->e_entry;
  // printf("entry is %x num is %d offset is %x\n", entry, hd->e_phnum,
  //        hd->e_phoff);
  Elf_Phdr *phs = (Elf_Phdr *)(date + hd->e_phoff);
  uintptr_t sz = 0;
  for (int i = 0; i < hd->e_phnum; i++) {
    Elf_Phdr ph = phs[i];
    if (ph.p_type == PT_LOAD) {
      // memmove((void *)ph.p_vaddr, date + ph.p_offset, ph.p_filesz);
      // memset((void *)(ph.p_vaddr + ph.p_filesz), 0, ph.p_memsz -
      // ph.p_filesz);
      Log("load %x  %x  %x  %x", ph.p_vaddr, ph.p_memsz, ph.p_offset,
          ph.p_filesz);
      // 分别记录着当前段的虚拟地址,虚拟地址大小,文件的偏移量,加载数据的大小
      uintptr_t vaddr = ph.p_vaddr, vsize = ph.p_memsz, f_offset = ph.p_offset,
                f_size = ph.p_filesz;
      assert(vaddr % PGSIZE == 0);
      uintptr_t size = 0;          // 记录了当前移动了多少的数据
      char *pos = date + f_offset; // 记录了移动到哪里了
      while (size < vsize) {
        char *page = new_page(1);
        if (size > f_size) { // 对照虚拟大小
          memset(page, 0, PGSIZE);
        } else if (size + PGSIZE > f_size) { // 对照临界情况
          memmove(page, pos, f_size % PGSIZE);
          memset(page + (f_size % PGSIZE), 0, PGSIZE - (f_size % PGSIZE));
        } else { // 直接copy
          memmove(page, pos, PGSIZE);
        }
        sz += PGSIZE;
        map(&pcb->as, (char *)vaddr, page, 0);
        pos += PGSIZE;
        vaddr += PGSIZE;
        size += PGSIZE;
      }
    }
  }
  // 记录当前进程的最大的虚拟地址空间大小
  pcb->max_brk = sz;
  // Log("loader max size is %p",sz);
  return entry;
}

uintptr_t naive_uload(PCB *pcb, const char *filename) {
  int fd = fs_open(filename, 0, 0);
  uintptr_t entry = load_segement(pcb, (char *)&ramdisk_start + fs_diskoff(fd));
  Log("Jump to entry = %p", entry);
  return entry;
}
