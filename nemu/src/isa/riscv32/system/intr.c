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

#include "../../monitor/sdb/sdb.h"
#include <isa.h>

// 设置异常号以及保存当前的pc值到mepc
// 返回异常入口地址
// mcause并不是直接的a7寄存器的值而是需要进一步的解释,怎么进行解释?
word_t isa_raise_intr(word_t NO, vaddr_t epc) {
  cpu.mepc = epc;
  cpu.mcause = 0xb;
  IFDEF(CONFIG_ETRACE, new_etrace(epc, NO));
  return cpu.mtvec;
}

word_t isa_query_intr() { return INTR_EMPTY; }
