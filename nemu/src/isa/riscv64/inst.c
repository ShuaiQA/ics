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
#include <cpu/cpu.h>
#include <cpu/decode.h>
#include <cpu/ifetch.h>
#include <stdint.h>

#define R(i) gpr(i)
#define Mr vaddr_read
#define Mw vaddr_write

enum {
  TYPE_I,
  TYPE_U,
  TYPE_S,
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

static void decode_operand(Decode *s, int *rd, word_t *src1, word_t *src2,
                           word_t *imm, int type) {
  uint32_t i = s->isa.inst.val;
  int rs1 = BITS(i, 19, 15);
  int rs2 = BITS(i, 24, 20);
  *rd = BITS(i, 11, 7);
  switch (type) {
  case TYPE_I:
    src1R();
    immI();
    break;
  case TYPE_U:
    immU();
    break;
  case TYPE_S:
    src1R();
    src2R();
    immS();
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
  INSTPAT("??????? ????? ????? ??? ????? 0110111", lui, U, R(rd) = imm); //
  INSTPAT("??????? ????? ????? ??? ????? 0010111", auipc, U,
          R(rd) = cpu.pc + imm);

  INSTPAT("??????? ????? ????? ??? ????? 1101111", jal, U, R(rd) = cpu.pc + 4;
          imm =
              ((BITS(imm, 30, 21)) | (BITS(imm, 20, 20) << 10) |
               (BITS(imm, 19, 12) << 11) | (SEXT(BITS(imm, 31, 31), 1) << 19));
          R(rd) = cpu.pc + 4; s->dnpc = cpu.pc + (imm << 1));

  INSTPAT("??????? ????? ????? 000 ????? 1100111", ret_jalr, I,
          s->dnpc = (src1 + imm) & ~1, R(rd) = cpu.pc + 4);
  // 跳转指令使用ra寄存器保存下一条返回指令
  INSTPAT("??????? ????? ????? 000 ????? 1100011", beq, S,
          imm = ((SEXT(BITS(imm, 11, 11), 1) << 11) | (BITS(imm, 0, 0) << 10) |
                 (BITS(imm, 10, 5) << 4) | BITS(imm, 4, 1));
          s->dnpc = (src1 == src2) ? (cpu.pc + (imm << 1)) : (cpu.pc + 4));
  INSTPAT("??????? ????? ????? 001 ????? 1100011", bne, S,
          imm = ((SEXT(BITS(imm, 11, 11), 1) << 11) | (BITS(imm, 0, 0) << 10) |
                 (BITS(imm, 10, 5) << 4) | BITS(imm, 4, 1));
          s->dnpc = (src1 != src2) ? (cpu.pc + (imm << 1)) : (cpu.pc + 4));
  INSTPAT("??????? ????? ????? 100 ????? 1100011", blt, S,
          imm = ((SEXT(BITS(imm, 11, 11), 1) << 11) | (BITS(imm, 0, 0) << 10) |
                 (BITS(imm, 10, 5) << 4) | BITS(imm, 4, 1));
          s->dnpc = ((sword_t)src1 < (sword_t)src2) ? (cpu.pc + (imm << 1))
                                                    : (cpu.pc + 4));
  INSTPAT("??????? ????? ????? 101 ????? 1100011", bge, S,
          imm = ((SEXT(BITS(imm, 11, 11), 1) << 11) | (BITS(imm, 0, 0) << 10) |
                 (BITS(imm, 10, 5) << 4) | BITS(imm, 4, 1));
          s->dnpc = ((sword_t)src1 >= (sword_t)src2) ? (cpu.pc + (imm << 1))
                                                     : (cpu.pc + 4));
  INSTPAT("??????? ????? ????? 110 ????? 1100011", bltu, S,
          imm = ((SEXT(BITS(imm, 11, 11), 1) << 11) | (BITS(imm, 0, 0) << 10) |
                 (BITS(imm, 10, 5) << 4) | BITS(imm, 4, 1));
          s->dnpc = (src1 < src2) ? (cpu.pc + (imm << 1)) : (cpu.pc + 4));
  INSTPAT("??????? ????? ????? 111 ????? 1100011", bgeu, S,
          imm = ((SEXT(BITS(imm, 11, 11), 1) << 11) | (BITS(imm, 0, 0) << 10) |
                 (BITS(imm, 10, 5) << 4) | BITS(imm, 4, 1));
          s->dnpc = (src1 >= src2) ? (cpu.pc + (imm << 1)) : (cpu.pc + 4));
  // 从内存中数据加载到寄存器中
  INSTPAT("??????? ????? ????? 000 ????? 0000011", lb, I,
          imm = Mr(src1 + imm, 1);
          R(rd) = imm | (SEXT(BITS(imm, 7, 7), 1) << 8));
  INSTPAT("??????? ????? ????? 001 ????? 0000011", lh, I,
          imm = Mr(src1 + imm, 2);
          R(rd) = imm | (SEXT(BITS(imm, 15, 15), 1) << 16));
  // 寄存器中的值加上寄存器获取地址,放到寄存器中
  INSTPAT("??????? ????? ????? 010 ????? 0000011", lw, I,
          R(rd) = (int64_t)(int32_t)Mr(src1 + imm, 4));
  INSTPAT("??????? ????? ????? 100 ????? 0000011", lbu, I,
          R(rd) = Mr(src1 + imm, 1));
  INSTPAT("??????? ????? ????? 101 ????? 0000011", lhu, I,
          R(rd) = Mr(src1 + imm, 2));
  // 寄存器中数据写回到内存中
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
  INSTPAT("??????? ????? ????? 100 ????? 0010011", xori, I,
          R(rd) = (src1 ^ imm));
  INSTPAT("??????? ????? ????? 110 ????? 0010011", ori, I,
          R(rd) = (src1 | imm));
  INSTPAT("??????? ????? ????? 111 ????? 0010011", andi, I,
          R(rd) = (src1 & imm));
  INSTPAT("0000000 ????? ????? 001 ????? 0010011", slli, I,
          R(rd) = (src1 << BITS(imm, 5, 0)));
  INSTPAT("0000000 ????? ????? 101 ????? 0010011", srli, I,
          R(rd) = (src1 >> BITS(imm, 5, 0)));
  INSTPAT("0100000 ????? ????? 101 ????? 0010011", srai, I,
          R(rd) = ((sword_t)src1 >> BITS(imm, 5, 0)));
  INSTPAT("0000000 ????? ????? 000 ????? 0110011", add, S, R(rd) = src1 + src2);
  INSTPAT("0100000 ????? ????? 000 ????? 0110011", sub, S, R(rd) = src1 - src2);
  INSTPAT("0000000 ????? ????? 001 ????? 0110011", sll, S,
          R(rd) = src1 << (src2 & 0b11111));
  INSTPAT("0000000 ????? ????? 010 ????? 0110011", slt, S,
          R(rd) = (sword_t)src1 < (sword_t)src2);
  INSTPAT("0000000 ????? ????? 011 ????? 0110011", sltu, S,
          R(rd) = src1 < src2);
  INSTPAT("0000000 ????? ????? 100 ????? 0110011", xor, S, R(rd) = src1 ^ src2);
  INSTPAT("0000000 ????? ????? 101 ????? 0110011", srl, S,
          R(rd) = src1 >> (src2 & 0b11111));
  INSTPAT("0100000 ????? ????? 101 ????? 0110011", sra, S,
          R(rd) = (sword_t)src1 >> (src2 & 0b11111));
  INSTPAT("0000000 ????? ????? 110 ????? 0110011", or, S, R(rd) = src1 | src2);
  INSTPAT("0000000 ????? ????? 111 ????? 0110011", and, S, R(rd) = src1 & src2);
  // super inst
  // ecall指令会将寄存器a7(异常号)和pc的值放到变量中跳转到
  // __am_asm_trap(void);汇编代码中,汇编代码主要的作用是,首先将寄存器中的数据保存到栈中,
  // 然后将mcause,mstatus,mepc变量保存到寄存器t0,t1,t2中,然后将t0,t1,t2保存到栈中,
  // 之后调用__am_irq_handle进行异常处理,之后进入do_event函数
  // 之后进行返回,将结构体c栈中的数据mepc,mcause传到t1,t2在将t1,t2数据传到mepc,mcause变量中
  INSTPAT("0000000 00000 00000 000 00000 1110011", ecall, I,
          s->dnpc = isa_raise_intr(cpu.gpr[17], cpu.pc););
  INSTPAT("0000000 00001 00000 000 00000 1110011", ebreak, N,
          NEMUTRAP(s->pc, R(10))); // R(10) is $a0

  // RV64I
  INSTPAT("??????? ????? ????? 110 ????? 0000011", lwu, I, R(rd) = imm + src1);
  INSTPAT("??????? ????? ????? 011 ????? 0000011", ld, I,
          R(rd) = Mr(src1 + imm, 8));
  INSTPAT("??????? ????? ????? 011 ????? 0100011", sd, S,
          Mw(src1 + imm, 8, src2));
  INSTPAT("000000? ????? ????? 001 ????? 0010011", slli, I,
          R(rd) = src1 << imm);
  INSTPAT("000000? ????? ????? 101 ????? 0010011", srli, I,
          R(rd) = src1 >> imm);
  INSTPAT("010000? ????? ????? 101 ????? 0010011", srai, I,
          R(rd) = src1 >> imm);
  // 将结果转换成32位,在符号扩展为64位
  INSTPAT("??????? ????? ????? 000 ????? 0011011", addiw, I,
          R(rd) = (int64_t)(int32_t)(src1 + imm));
  INSTPAT("0000000 ????? ????? 001 ????? 0011011", slliw, I,
          R(rd) = src1 << imm);
  INSTPAT("0000000 ????? ????? 001 ????? 0011011", srliw, I,
          R(rd) = src1 >> imm);
  INSTPAT("0100000 ????? ????? 101 ????? 0011011", sraiw, I,
          R(rd) = src1 >> imm);
  INSTPAT("0000000 ????? ????? 000 ????? 0111011", addw, S,
          R(rd) = src1 + src2);
  INSTPAT("0100000 ????? ????? 000 ????? 0111011", subw, S,
          R(rd) = src1 - src2);
  INSTPAT("0000000 ????? ????? 001 ????? 0111011", sllw, S,
          R(rd) = src1 << src2);
  INSTPAT("0000000 ????? ????? 101 ????? 0111011", srlw, S,
          R(rd) = src1 >> (src2 & 0b11111));
  INSTPAT("0100000 ????? ????? 101 ????? 0111011", sraw, S,
          R(rd) = src1 >> (src2 & 0b11111));

  INSTPAT("0000001 ????? ????? 000 ????? 0110011", mul, S,
          R(rd) = (sword_t)src1 * (sword_t)src2);
  INSTPAT("0000001 ????? ????? 001 ????? 0110011", mulh, S,
          long long c = (long long)(sword_t)src1 * (long long)(sword_t)src2;
          R(rd) = (sword_t)(c >> 32););
  INSTPAT("0000001 ????? ????? 011 ????? 0110011", mulhu, S,
          long long c = (long long)src1 * (long long)src2;
          R(rd) = (word_t)(c >> 32));
  INSTPAT("0000001 ????? ????? 010 ????? 0110011", mulhsu, S,
          long long c = (long long)(sword_t)src1 * (long long)src2;
          R(rd) = (word_t)(c >> 32));

  INSTPAT("0000001 ????? ????? 100 ????? 0110011", div, S,
          R(rd) = (sword_t)src1 / (sword_t)src2);
  INSTPAT("0000001 ????? ????? 101 ????? 0110011", divu, S,
          R(rd) = src1 / src2);
  INSTPAT("0000001 ????? ????? 110 ????? 0110011", rem, S,
          R(rd) = (sword_t)src1 % (sword_t)src2);
  INSTPAT("0000001 ????? ????? 111 ????? 0110011", remu, S,
          R(rd) = src1 % src2);

  // INSTPAT("0011010 00010 00000 010 ????? 1110011", csrr_mcause, I,
  //         R(rd) = cpu.mcause;);
  // INSTPAT("0011000 00000 00000 010 ????? 1110011", csrr_mstatus, I,
  //         R(rd) = cpu.mstatus;);
  // INSTPAT("0011010 00001 00000 010 ????? 1110011", csrr_mepc, I,
  //         R(rd) = cpu.mepc;);
  // INSTPAT("0011000 00000 00110 001 00000 1110011", csrw_mstatus, I,
  //         cpu.mstatus = src1);
  // INSTPAT("0011010 00001 00111 001 00000 1110011", wmepc, I, cpu.mepc =
  // src1);
  // // 存储80000550 <__am_asm_trap>地址到sr.mtvec变量中
  // INSTPAT("0011000 00101 01111 001 00000 1110011", csrw_mtvec, I,
  //         cpu.mtvec = src1;);
  // INSTPAT("0001100 00000 00000 010 ????? 1110011", csrr_satp, I,
  //         R(rd) = cpu.satp;);
  // INSTPAT("0001100 00000 ????? 001 00000 1110011", csrw_satp, I,
  //         cpu.satp = src1;
  //         Log("change satp " FMT_WORD, cpu.satp););
  // INSTPAT("0011000 00010 00000 000 00000 1110011", mret, I,
  //         s->dnpc = cpu.mepc;);
  INSTPAT("??????? ????? ????? ??? ????? ???????", inv, N, INV(s->pc));
  INSTPAT_END();

  R(0) = 0; // reset $zero to 0

  return 0;
}

int isa_exec_once(Decode *s) {
  s->isa.inst.val = inst_fetch(&s->snpc, 4);
  return decode_exec(s);
}
