#include <elf.h>
#include <fs.h>
#include <proc.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __LP64__
#define Elf_Ehdr Elf64_Ehdr
#define Elf_Phdr Elf64_Phdr
#else
#define Elf_Ehdr Elf32_Ehdr
#define Elf_Phdr Elf32_Phdr
#endif

// 根据字符数组获取需要加载到内容中
uintptr_t load_segement(char *date) {
  Elf_Ehdr *hd = (Elf_Ehdr *)date;
  assert(*(uint32_t *)hd->e_ident == 0x464c457f);
  uintptr_t entry = hd->e_entry;
  // printf("entry is %x num is %d offset is %x\n", entry, hd->e_phnum,
  //        hd->e_phoff);
  Elf_Phdr *phs = (Elf_Phdr *)(date + hd->e_phoff);
  for (int i = 0; i < hd->e_phnum; i++) {
    Elf_Phdr ph = phs[i];
    if (ph.p_type == PT_LOAD) {
      // printf("%x  %x  %x\n", ph.p_vaddr, ph.p_offset, ph.p_filesz);
      memmove((void *)ph.p_vaddr, date + ph.p_offset, ph.p_filesz);
      char *next = (char *)malloc(ph.p_memsz - ph.p_filesz);
      memset(next, 0, ph.p_memsz - ph.p_filesz);
      memmove((void *)(ph.p_vaddr + ph.p_filesz), next,
              ph.p_memsz - ph.p_filesz);
    }
  }
  return entry;
}

// 根据文件的名字
// static uintptr_t loader(PCB *pcb, const char *filename) {
//   int fd = fs_open(filename, 0, 0);
//   size_t size = getfilesize(fd);
//   fs_lseek(fd, 0, SEEK_SET);
//   char *date = malloc(size);
//   fs_read(fd, date, size);
//   uintptr_t entry = load_segement(date);
//   fs_close(fd);
//   return entry;
// }

void naive_uload(PCB *pcb, const char *filename) {
  // uintptr_t entry = loader(pcb, "/bin/bmp-test");
  char buf[6000];
  ramdisk_read(buf, 0, 5676);
  uintptr_t entry = load_segement(buf);
  Log("Jump to entry = %p", entry);
  ((void (*)())entry)();
}
