#ifndef NEMU_H__
#define NEMU_H__

#include <klib-macros.h>

#include ISA_H // the macro `ISA_H` is defined in CFLAGS
               // it will be expanded as "x86/x86.h", "mips/mips32.h", ...

#if defined(__ISA_X86__)
#define nemu_trap(code) asm volatile("int3" : : "a"(code))
#elif defined(__ISA_MIPS32__)
#define nemu_trap(code) asm volatile("move $v0, %0; sdbbp" : : "r"(code))
#elif defined(__ISA_RISCV32__) || defined(__ISA_RISCV64__)
#define nemu_trap(code) asm volatile("mv a0, %0; ebreak" : : "r"(code))
#elif defined(__ISA_LOONGARCH32R__)
#define nemu_trap(code) asm volatile("move $a0, %0; break 0" : : "r"(code))
#elif
#error unsupported ISA __ISA__
#endif

#if defined(__ARCH_X86_NEMU)
#define DEVICE_BASE 0x0
#else
#define DEVICE_BASE 0xa0000000
#endif

#define MMIO_BASE 0xa0000000

// 相关设备的寄存器管理内存映射地址
#define SERIAL_PORT (DEVICE_BASE + 0x00003f8)
#define KBD_ADDR (DEVICE_BASE + 0x0000060)
#define RTC_ADDR (DEVICE_BASE + 0x0000048)
#define VGACTL_ADDR (DEVICE_BASE + 0x0000100)
#define AUDIO_ADDR (DEVICE_BASE + 0x0000200)
#define DISK_ADDR (DEVICE_BASE + 0x0000300)
// 下面的地址是屏幕、音频、磁盘的存储地址
#define FB_ADDR (MMIO_BASE + 0x1000000)
#define AUDIO_SBUF_ADDR (MMIO_BASE + 0x1200000)
#define DISK_BUF_ADDR (MMIO_BASE + 0x1400000)

extern char _pmem_start;
#define PMEM_SIZE (128 * 1024 * 1024)
#define PMEM_END ((uintptr_t)&_pmem_start + PMEM_SIZE)
#define NEMU_PADDR_SPACE                                                       \
  RANGE(&_pmem_start, PMEM_END), RANGE(FB_ADDR, FB_ADDR + 0x200000),           \
      RANGE(MMIO_BASE, MMIO_BASE + 0x1000),                                    \
      RANGE(DISK_BUF_ADDR, DISK_BUF_ADDR + 0x1000)

typedef uintptr_t PTE;
#define PXMASK 0x1FF // 9 bits
#define PGSHIFT 12   // bits of offset within a page
#define PXSHIFT(level) (PGSHIFT + (9 * (level)))
#define PX(level, va) ((((uintptr_t)(va)) >> PXSHIFT(level)) & PXMASK)
#define PTE2PA(pte) (((pte) >> 10) << 12) // 根据页表项获取物理地址
#define PA2PTE(pa) ((((uintptr_t)pa) >> 12) << 10)
typedef uintptr_t *pagetable_t; // 512 PTEs

#define PGSIZE 4096

#endif
