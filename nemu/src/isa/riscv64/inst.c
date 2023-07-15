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
#include "isa.h"
#include "local-include/reg.h"
#include "macro.h"
#include <cpu/cpu.h>
#include <cpu/decode.h>
#include <cpu/ifetch.h>
#include <stdint.h>

#define R(i) gpr(i)
#define RM(i) mcsr(i)
#define Mr vaddr_read
#define Mw vaddr_write

enum {
  TYPE_I,
  TYPE_U,
  TYPE_S,
  TYPE_R,
  TYPE_B,
  TYPE_J,
  TYPE_N, // none
};

#define src1R()                                                                \
  do {                                                                         \
    *src1 = R(rs1);                                                            \
  } while (0)
#define src2R()                                                                \
  do {                                                                         \
    *src2 = R(rs2);                                                            \
  } while (0)
#define immI()                                                                 \
  do {                                                                         \
    *imm = SEXT(BITS(i, 31, 20), 12);                                          \
  } while (0)
#define immU()                                                                 \
  do {                                                                         \
    *imm = SEXT(BITS(i, 31, 12), 20) << 12;                                    \
  } while (0)
#define immS()                                                                 \
  do {                                                                         \
    *imm = (SEXT(BITS(i, 31, 25), 7) << 5) | BITS(i, 11, 7);                   \
  } while (0)
#define immJ()                                                                 \
  do {                                                                         \
    *imm = (SEXT(BITS(i, 31, 31), 1) << 20) | (BITS(i, 19, 12) << 12) |        \
           (BITS(i, 20, 20) << 11) | (BITS(i, 30, 21) << 1);                   \
  } while (0)
#define immB()                                                                 \
  do {                                                                         \
    *imm = (SEXT(BITS(i, 31, 31), 1) << 12) | (BITS(i, 30, 25) << 5) |         \
           (BITS(i, 11, 8) << 1) | (BITS(i, 7, 7) << 11);                      \
  } while (0)

// imm -> rd , rs1 -> imm
void csrrw(word_t imm, word_t src1, word_t rd) {
  if (rd != 0) {
    R(rd) = RM(imm);
  }
  RM(imm) = src1;
}

void csrrs(word_t imm, word_t src1, word_t rd) {
  R(rd) = RM(imm);
  RM(imm) = RM(imm) | src1;
}

static void decode_operand(Decode *s, int *rd, word_t *src1, word_t *src2,
                           word_t *imm, int type) {
  uint32_t i = s->isa.inst.val;
  int rs1 = BITS(i, 19, 15);
  int rs2 = BITS(i, 24, 20);
  *rd = BITS(i, 11, 7);
  switch (type) {
  case TYPE_R:
    src1R();
    src2R();
    break;
  case TYPE_I:
    src1R();
    immI();
    break;
  case TYPE_U:
    immU();
    break;
  case TYPE_J:
    immJ();
    break;
  case TYPE_S:
    src1R();
    src2R();
    immS();
    break;
  case TYPE_B:
    src1R();
    src2R();
    immB();
    break;
  }
}

static int decode_exec(Decode *s) {
  int rd = 0;
  word_t src1 = 0, src2 = 0, imm = 0;
  s->dnpc = s->snpc;

#define INSTPAT_INST(s) ((s)->isa.inst.val)
#define INSTPAT_MATCH(s, name, type, ... /* execute body */)                   \
  {                                                                            \
    decode_operand(s, &rd, &src1, &src2, &imm, concat(TYPE_, type));           \
    __VA_ARGS__;                                                               \
  }

  INSTPAT_START();
  INSTPAT("??????? ????? ????? ??? ????? 0110111", lui, U, R(rd) = imm);
  INSTPAT("??????? ????? ????? ??? ????? 0010111", auipc, U,
          R(rd) = cpu.pc + imm);
  INSTPAT("??????? ????? ????? ??? ????? 1101111", jal, J, R(rd) = cpu.pc + 4;
          R(rd) = cpu.pc + 4; s->dnpc = cpu.pc + imm);
  INSTPAT("??????? ????? ????? 000 ????? 1100111", ret_jalr, I,
          s->dnpc = (src1 + imm), R(rd) = cpu.pc + 4);
  // 跳转指令使用ra寄存器保存下一条返回指令
  INSTPAT("??????? ????? ????? 000 ????? 1100011", beq, B,
          s->dnpc = (src1 == src2) ? (cpu.pc + imm) : (cpu.pc + 4));
  INSTPAT("??????? ????? ????? 001 ????? 1100011", bne, B,
          s->dnpc = (src1 != src2) ? (cpu.pc + imm) : (cpu.pc + 4));
  INSTPAT("??????? ????? ????? 100 ????? 1100011", blt, B,
          s->dnpc =
              ((sword_t)src1 < (sword_t)src2) ? (cpu.pc + imm) : (cpu.pc + 4));
  INSTPAT("??????? ????? ????? 101 ????? 1100011", bge, B,
          s->dnpc =
              ((sword_t)src1 >= (sword_t)src2) ? (cpu.pc + imm) : (cpu.pc + 4));
  INSTPAT("??????? ????? ????? 110 ????? 1100011", bltu, B,
          s->dnpc = (src1 < src2) ? (cpu.pc + imm) : (cpu.pc + 4));
  INSTPAT("??????? ????? ????? 111 ????? 1100011", bgeu, B,
          s->dnpc = (src1 >= src2) ? (cpu.pc + imm) : (cpu.pc + 4));

  // 从内存中数据加载到寄存器中
  INSTPAT("??????? ????? ????? 000 ????? 0000011", lb, I,
          R(rd) = SEXT(Mr(src1 + imm, 1), 8));
  INSTPAT("??????? ????? ????? 001 ????? 0000011", lh, I,
          R(rd) = SEXT(Mr(src1 + imm, 2), 16));
  INSTPAT("??????? ????? ????? 010 ????? 0000011", lw, I,
          R(rd) = SEXT(Mr(src1 + imm, 4), 32));
  INSTPAT("??????? ????? ????? 100 ????? 0000011", lbu, I,
          R(rd) = Mr(src1 + imm, 1));
  INSTPAT("??????? ????? ????? 101 ????? 0000011", lhu, I,
          R(rd) = Mr(src1 + imm, 2));
  INSTPAT("??????? ????? ????? 000 ????? 0100011", sb, S,
          Mw(src1 + imm, 1, src2));
  INSTPAT("??????? ????? ????? 001 ????? 0100011", sh, S,
          Mw(src1 + imm, 2, src2));
  INSTPAT("??????? ????? ????? 010 ????? 0100011", sw, S,
          Mw(src1 + imm, 4, src2));
  INSTPAT("??????? ????? ????? 000 ????? 0010011", addi_li, I,
          R(rd) = imm + src1);
  INSTPAT("??????? ????? ????? 010 ????? 0010011", slti, I,
          R(rd) = ((sword_t)src1 < (sword_t)imm));
  INSTPAT("??????? ????? ????? 011 ????? 0010011", sltiu, I,
          R(rd) = (src1 < imm));
  INSTPAT("??????? ????? ????? 100 ????? 0010011", xori, I, R(rd) = src1 ^ imm);
  INSTPAT("??????? ????? ????? 110 ????? 0010011", ori, I, R(rd) = src1 | imm);
  INSTPAT("??????? ????? ????? 111 ????? 0010011", andi, I, R(rd) = src1 & imm);
  INSTPAT("0000000 ????? ????? 001 ????? 0010011", slli, I,
          R(rd) = (src1 << BITS(imm, 5, 0)));
  INSTPAT("0000000 ????? ????? 101 ????? 0010011", srli, I,
          R(rd) = (src1 >> BITS(imm, 5, 0)));
  INSTPAT("0100000 ????? ????? 101 ????? 0010011", srai, I,
          R(rd) = ((sword_t)src1 >> BITS(imm, 5, 0)));

  INSTPAT("0000000 ????? ????? 000 ????? 0110011", add, R, R(rd) = src1 + src2);
  INSTPAT("0100000 ????? ????? 000 ????? 0110011", sub, R, R(rd) = src1 - src2);
  INSTPAT("0000000 ????? ????? 001 ????? 0110011", sll, R,
          R(rd) = src1 << BITS(src2, 5, 0));
  INSTPAT("0000000 ????? ????? 010 ????? 0110011", slt, R,
          R(rd) = (sword_t)src1 < (sword_t)src2);
  INSTPAT("0000000 ????? ????? 011 ????? 0110011", sltu, R,
          R(rd) = src1 < src2);
  INSTPAT("0000000 ????? ????? 100 ????? 0110011", xor, R, R(rd) = src1 ^ src2);
  INSTPAT("0000000 ????? ????? 101 ????? 0110011", srl, R,
          R(rd) = src1 >> BITS(src2, 5, 0));
  INSTPAT("0100000 ????? ????? 101 ????? 0110011", sra, S,
          R(rd) = (sword_t)src1 >> BITS(src2, 5, 0));
  INSTPAT("0000000 ????? ????? 110 ????? 0110011", or, S, R(rd) = src1 | src2);
  INSTPAT("0000000 ????? ????? 111 ????? 0110011", and, S, R(rd) = src1 & src2);

  // super inst
  // ecall指令会将寄存器a7(异常号)和pc的值放到变量中跳转到
  // __am_asm_trap(void);汇编代码中,汇编代码主要的作用是,首先将寄存器中的数据保存到栈中,
  // 然后将mcause,mstatus,mepc变量保存到寄存器t0,t1,t2中,然后将t0,t1,t2保存到栈中,
  // 之后调用__am_irq_handle进行异常处理,之后进入do_event函数
  // 之后进行返回,将结构体c栈中的数据mepc,mcause传到t1,t2在将t1,t2数据传到mepc,mcause变量中
  INSTPAT("0000000 00000 00000 000 00000 1110011", ecall, I,
          s->dnpc = isa_raise_intr(0xb, cpu.pc););
  INSTPAT("0000000 00001 00000 000 00000 1110011", ebreak, N,
          NEMUTRAP(s->pc, R(10))); // R(10) is $a0

  // RV64I
  INSTPAT("??????? ????? ????? 110 ????? 0000011", lwu, I,
          R(rd) = Mr(src1 + imm, 4));
  INSTPAT("??????? ????? ????? 011 ????? 0000011", ld, I,
          R(rd) = Mr(src1 + imm, 8));
  INSTPAT("??????? ????? ????? 011 ????? 0100011", sd, S,
          Mw(src1 + imm, 8, src2));
  INSTPAT("000000? ????? ????? 001 ????? 0010011", slli, I,
          R(rd) = src1 << BITS(imm, 5, 0));
  INSTPAT("000000? ????? ????? 101 ????? 0010011", srli, I,
          R(rd) = src1 >> BITS(imm, 5, 0));
  INSTPAT("010000? ????? ????? 101 ????? 0010011", srai, I,
          R(rd) = (sword_t)src1 >> BITS(imm, 5, 0));
  // 将结果转换成32位,在符号扩展为64位
  INSTPAT("??????? ????? ????? 000 ????? 0011011", addiw, I,
          R(rd) = SEXT((uint32_t)src1 + (uint32_t)imm, 32));
  INSTPAT("0000000 ????? ????? 001 ????? 0011011", slliw, I,
          R(rd) = SEXT((uint32_t)src1 << BITS(imm, 4, 0), 32));
  INSTPAT("0000000 ????? ????? 101 ????? 0011011", srliw, I,
          R(rd) = SEXT((uint32_t)src1 >> BITS(imm, 4, 0), 32));
  INSTPAT("0100000 ????? ????? 101 ????? 0011011", sraiw, I,
          R(rd) = SEXT((int32_t)src1 >> BITS(imm, 4, 0), 32));
  INSTPAT("0000000 ????? ????? 000 ????? 0111011", addw, R,
          R(rd) = SEXT((uint32_t)src1 + (uint32_t)src2, 32));
  INSTPAT("0100000 ????? ????? 000 ????? 0111011", subw, R,
          R(rd) = SEXT((uint32_t)src1 - (uint32_t)src2, 32));
  INSTPAT("0000000 ????? ????? 001 ????? 0111011", sllw, R,
          R(rd) = SEXT((uint32_t)src1 << BITS(src2, 4, 0), 32));
  INSTPAT("0000000 ????? ????? 101 ????? 0111011", srlw, R,
          R(rd) = SEXT((uint32_t)src1 >> BITS(src2, 4, 0), 32));
  INSTPAT("0100000 ????? ????? 101 ????? 0111011", sraw, R,
          R(rd) = SEXT((int32_t)src1 >> BITS(src2, 4, 0), 32));
  // 32M
  __int128_t temp = 0;
  INSTPAT("0000001 ????? ????? 000 ????? 0110011", mul, R, R(rd) = src1 * src2);
  INSTPAT("0000001 ????? ????? 001 ????? 0110011", mulh, R,
          temp = (sword_t)src1 * (sword_t)src2, R(rd) = temp >> 64);
  INSTPAT("0000001 ????? ????? 010 ????? 0110011", mulhsu, R,
          temp = (sword_t)src1 * (word_t)src2, R(rd) = temp >> 64);
  INSTPAT("0000001 ????? ????? 011 ????? 0110011", mulhu, R,
          temp = (word_t)src1 * (word_t)src2, R(rd) = temp >> 64);
  INSTPAT("0000001 ????? ????? 100 ????? 0110011", div, R,
          R(rd) = (sword_t)src1 / (sword_t)src2);
  INSTPAT("0000001 ????? ????? 101 ????? 0110011", divu, R,
          R(rd) = (word_t)src1 / (word_t)src2);
  INSTPAT("0000001 ????? ????? 110 ????? 0110011", rem, R,
          R(rd) = (sword_t)src1 % (sword_t)src2);
  INSTPAT("0000001 ????? ????? 111 ????? 0110011", remu, R,
          R(rd) = (word_t)src1 % (word_t)src2);
  // 64M
  INSTPAT("0000001 ????? ????? 000 ????? 0111011", mulw, R,
          R(rd) = SEXT(BITS((int32_t)src1 * (int32_t)src2, 31, 0), 32));
  INSTPAT("0000001 ????? ????? 100 ????? 0111011", divw, R,
          R(rd) = SEXT((int32_t)src1 / (int32_t)src2, 32));
  INSTPAT("0000001 ????? ????? 101 ????? 0111011", divuw, R,
          R(rd) = SEXT((uint32_t)src1 / (uint32_t)src2, 32));
  INSTPAT("0000001 ????? ????? 110 ????? 0111011", remw, R,
          R(rd) = SEXT((int32_t)src1 % (int32_t)src2, 32));
  INSTPAT("0000001 ????? ????? 111 ????? 0111011", remuw, R,
          R(rd) = SEXT((uint32_t)src1 % (uint32_t)src2, 32));

  // 001101000001 00000 010 00111 1110011 csrr	t2,mepc
  INSTPAT("???????????? ????? 001 ????? 1110011", csrrw, I,
          csrrw(imm, src1, rd));

  // 0011 0100 0010 0000 0010 0010 1111 0011  csrrs   t0, mcause, zeroe8
  INSTPAT("???????????? ????? 010 ????? 1110011", csrrs, I,
          csrrs(imm, src1, rd));

  // INSTPAT("0011010 00010 00000 010 ????? 1110011", csrr_mcause, I,
  //         R(rd) = cpu.mcause;);
  // INSTPAT("0011000 00000 00000 010 ????? 1110011", csrr_mstatus, I,
  //         R(rd) = cpu.mstatus;);
  // INSTPAT("0011010 00001 00000 010 ????? 1110011", csrr_mepc, I,
  //         R(rd) = cpu.mepc;);
  // // 存储80000550 <__am_asm_trap>地址到sr.mtvec变量中
  // INSTPAT("0001100 00000 00000 010 ????? 1110011", csrr_satp, I,
  //         R(rd) = cpu.satp;);
  // INSTPAT("0011000 00010 00000 000 00000 1110011", mret, I,
  //         s->dnpc = cpu.mepc;);
  INSTPAT("??????? ????? ????? ??? ????? ???????", inv, N, INV(s->pc));
  INSTPAT_END();

  R(0) = 0; // reset $zero to 0

  return 0;
}

int isa_exec_once(Decode *s) { return decode_exec(s); }
