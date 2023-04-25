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
#include <cpu/difftest.h>
#include <isa.h>

bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc) {
  bool ans = true;
  if (ref_r->pc != cpu.pc) {
    printf("pc is 0x%x fact is 0x%x", ref_r->pc, cpu.pc);
    ans = false;
  }
  for (int i = 0; i < 32; i = i + 1) {
    if (ref_r->gpr[i] != cpu.gpr[i]) {
      printf("pc is 0x%x reg %s expect is 0x%x fact is 0x%x\n", cpu.pc,
             reg_name(i, 0), ref_r->gpr[i], cpu.gpr[i]);
      ans = false;
    }
  }
  return ans;
}

void isa_difftest_attach() {}
