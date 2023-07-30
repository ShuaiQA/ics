/***************************************************************************************
 * Copyright (c) 2014-2022 Zihao Yu, Nanjing University
 *
 * NEMU is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan
 *PSL v2. You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 *KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 *NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 ***************************************************************************************/

#include "../local-include/reg.h"
#include "common.h"
#include "debug.h"
#include <isa.h>
#include <memory/paddr.h>
#include <memory/vaddr.h>

typedef word_t *pagetable_t; // 512 PTEs
typedef word_t PTE;
#define PXMASK 0x1FF // 9 bits
#define PGSHIFT 12   // bits of offset within a page
#define PXSHIFT(level) (PGSHIFT + (9 * (level)))
#define PX(level, va) ((((uintptr_t)(va)) >> PXSHIFT(level)) & PXMASK)
#define PTE2PA(pte) (((pte) >> 10) << 12) // 根据页表项获取物理地址
#define PA2PTE(pa) ((((uintptr_t)pa) >> 12) << 10)
typedef uintptr_t *pagetable_t; // 512 PTEs
#define PTE_V 0x01
#define PTE_R 0x02
#define PTE_W 0x04
#define PTE_X 0x08
#define PTE_U 0x10
#define PTE_A 0x40
#define PTE_D 0x80

// 将虚拟地址转换成物理地址
paddr_t isa_mmu_translate(vaddr_t vaddr) {
  pagetable_t pagetable = (pagetable_t)cpu.mcsr[5];
  for (int level = 2; level > 0; level--) {
    PTE *pte = &pagetable[PX(level, vaddr)];
    if (*pte & PTE_V) {
      pagetable = (pagetable_t)PTE2PA(*pte);
    } else {
      Assert(0, "PTE_V error");
    }
  }
  return PTE2PA(pagetable[PX(0, vaddr)]);
}

// 查看是否需要进行转换
int isa_mmu_check(vaddr_t vaddr) {
  if (1ul << 63 & MCSR(satp)) {
    Log("tran");
  }
  return (1ul << 63 & MCSR(satp)) ? MMU_TRANSLATE : MMU_DIRECT;
}
