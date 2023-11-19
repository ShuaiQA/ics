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

#include "debug.h"
#include "macro.h"
#include <../local-include/reg.h>
#include <common.h>
#include <isa.h>

word_t isa_raise_intr(word_t NO, vaddr_t epc) {
  MCSR(mepc) = epc;
  // ret返回之后的值,ecall之后保存在ra寄存器中
  // Log("ecall " FMT_WORD " save next pc is " FMT_WORD, cpu.gpr[17],
  // cpu.gpr[1]);
  MCSR(mcause) = 0xb;
  IFDEF(CONFIG_ETRACE, new_etrace());
  IFDEF(CONFIG_CONTEXT, Log("ecall context"));
  IFDEF(CONFIG_CONTEXT, isa_reg_display());
  return MCSR(mtvec);
}

word_t isa_query_intr() { return INTR_EMPTY; }
