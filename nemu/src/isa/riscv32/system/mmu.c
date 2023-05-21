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
#include <assert.h>
#include <isa.h>
#include <memory/paddr.h>
#include <memory/vaddr.h>
#include <stdint.h>
#include <stdio.h>

// https://zhuanlan.zhihu.com/p/61430196(介绍了相关的手册)
// 将虚拟地址翻译成对应的物理地址
paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type) {
  if (type == MMU_DIRECT) {
    return vaddr;
  }
  Log("vaddr is " FMT_WORD, vaddr);
  // 查看ab向寄存中写的数据发现需要左移12位找到对应的页目录的地址
  word_t page_dir = cpu.satp << 12; // 找到对应的页目录的物理地址
  Log("dir is " FMT_WORD, page_dir);
  word_t pte =
      paddr_read(page_dir + (vaddr >> 22) * 4, 4); // 找到页目录中的页表项
  word_t next_page = pte & 0xfffff000; // 根据页表项的获取二级页表的物理地址
  Log("table is " FMT_WORD, next_page);
  word_t next_pte = paddr_read(next_page + (vaddr << 10 >> 22) * 4,
                               4); // 获取二级页表中的页表项
  vaddr_t ret = (next_pte & 0xfffff000) + (vaddr & 0xfff);
  Log("vaddr is " FMT_WORD "ret is " FMT_WORD, vaddr, ret);
  // Assert(vaddr == ret, "vaddr is " FMT_WORD "  cnt is %d\n", vaddr, cnt);
  return ret; // 获取物理地址
}
