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

#include "common.h"
#include "debug.h"
#include <isa.h>
#include <memory/paddr.h>

word_t vaddr_ifetch(vaddr_t addr, int len) { return paddr_read(addr, len); }

// 先翻译成物理地址在进行读或者写
word_t vaddr_read(vaddr_t addr, int len) {
  Log("read addr is " FMT_PADDR, addr);
  paddr_t paddr = isa_mmu_translate(addr, len, cpu.satp >> 31);
  Log("p addr is " FMT_PADDR, paddr);
  return paddr_read(paddr, len);
}

void vaddr_write(vaddr_t addr, int len, word_t data) {
  Log("write addr is " FMT_PADDR, addr);
  paddr_t paddr = isa_mmu_translate(addr, len, cpu.satp >> 31);
  Log("p addr is" FMT_PADDR, paddr);
  paddr_write(paddr, len, data);
}
