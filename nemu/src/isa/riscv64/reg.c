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
#include <isa.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

const char *regs[] = {"$0", "ra", "sp",  "gp",  "tp", "t0", "t1", "t2",
                      "s0", "s1", "a0",  "a1",  "a2", "a3", "a4", "a5",
                      "a6", "a7", "s2",  "s3",  "s4", "s5", "s6", "s7",
                      "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"};

// mtvec : 0x305
enum { mtvec = 0x305 };
const char *mcsr[] = {"mstatus", "mtvec", "mepc", "mcause"};

void wmcsr(word_t pos, word_t val) {
  Log("pos is" FMT_WORD ",val is " FMT_WORD, pos, val);
  switch (pos) {
  case mtvec:
    cpu.mcsr[2] = val;
    break;
  default:
    Log("error");
  }
}

word_t rmscr(word_t pos) {
  int val = 0;
  switch (pos) {
  case mtvec:
    val = cpu.mcsr[2];
    break;
  default:
    Log("error");
  }
  return val;
}

void isa_reg_display() {
  printf("pc\t" FMT_WORD "\n", cpu.pc);
  for (int i = 0; i < 32; i++) {
    printf("%s\t" FMT_WORD "\t", regs[i], cpu.gpr[i]);
    if (i % 4 == 0) {
      printf("\n");
    }
  }
  for (int i = 0; i < 4; i++) {
    printf("%s\t" FMT_WORD "\t", mcsr[i], cpu.gpr[i]);
    if (i % 4 == 0) {
      printf("\n");
    }
  }
}

word_t isa_reg_str2val(const char *s, bool *success) {
  if (strcmp(s, "pc") == 0) {
    *success = true;
    return cpu.pc;
  }
  int pos = -1;
  for (int i = 0; i < 32; i++) {
    if (strcmp(s, regs[i]) == 0) {
      pos = i;
      break;
    }
  }
  for (int i = 0; i < 4; i++) {
    if (strcmp(s, mcsr[i]) == 0) {
      pos = i;
      break;
    }
  }
  if (pos == -1) {
    *success = false;
    Log("查找regs字符串错误");
    return 0;
  }
  *success = true;
  return cpu.gpr[pos];
}
