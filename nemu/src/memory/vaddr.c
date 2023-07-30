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

// 根据传入的地址返回对应的物理地址
static word_t paddr(vaddr_t addr) {
  int c = isa_mmu_check(addr);
  switch (c) {
  case MMU_DIRECT:
    return addr;
  case MMU_TRANSLATE:
    return isa_mmu_translate(addr);
  default:
    Assert(0, "not implement");
  }
}

word_t vaddr_ifetch(vaddr_t addr, int len) {
  return paddr_read(paddr(addr), len);
}

word_t vaddr_read(vaddr_t addr, int len) {
  return paddr_read(paddr(addr), len);
}

void vaddr_write(vaddr_t addr, int len, word_t data) {
  paddr_write(paddr(addr), len, data);
}
