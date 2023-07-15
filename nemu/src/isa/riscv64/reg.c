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

#include "local-include/reg.h"
#include "common.h"
#include "debug.h"
#include <assert.h>
#include <isa.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

const char *regs[] = {"$0", "ra", "sp",  "gp",  "tp", "t0", "t1", "t2",
                      "s0", "s1", "a0",  "a1",  "a2", "a3", "a4", "a5",
                      "a6", "a7", "s2",  "s3",  "s4", "s5", "s6", "s7",
                      "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"};

enum { mtvec = 0x305, mcause = 0x300 };
const char *mcsr[] = {"mstatus", "mtvec", "mepc", "mcause"};

size_t mcsrpos(word_t num) {
  Log("pos is" FMT_WORD, num);
  size_t pos = -1;
  switch (pos) {
  case mtvec:
    pos = 1;
    break;
  case mcause:
    pos = 3;
    break;
  }
  return pos;
}

void isa_reg_display() {
  printf("pc\t" FMT_WORD "\n", cpu.pc);
  for (int i = 0; i < 32; i++) {
    printf("%s\t" FMT_WORD "\t", regs[i], cpu.gpr[i]);
    if ((i + 1) % 4 == 0) {
      printf("\n");
    }
  }
  for (int i = 0; i < 4; i++) {
    printf("%s\t" FMT_WORD "\t", mcsr[i], cpu.mcsr[i]);
    if ((i + 1) % 4 == 0) {
      printf("\n");
    }
  }
}

word_t isa_reg_str2val(const char *s, bool *success) {
  if (strcmp(s, "pc") == 0) {
    *success = true;
    return cpu.pc;
  }
  for (int i = 0; i < 32; i++) {
    if (strcmp(s, regs[i]) == 0) {
      *success = true;
      return cpu.gpr[i];
    }
  }
  for (int i = 0; i < 4; i++) {
    if (strcmp(s, mcsr[i]) == 0) {
      *success = true;
      return cpu.mcsr[i];
    }
  }
  Log("查找regs字符串错误");
  *success = true;
  return 0;
}
