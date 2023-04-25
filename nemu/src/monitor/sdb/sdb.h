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

#ifndef __SDB_H__
#define __SDB_H__

#include <common.h>

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  word_t value;  // 保存older_value
  char what[40]; // 保存监视的是什么名字
} WP;

// 输出所有的断点集合   info w
void printWP();
// 创建一个断点         w $pc
WP *new_wp(word_t value, char *what);
// 释放一个断点         d 1
void free_wp(int no);
// 查看当前的值是否与断点值不一样    cpu_exec
bool find_watch();

typedef struct isa {
  word_t isa_val;
  word_t pc;
  char action[50];
} ISAbuf;

// cpu_exec 将最近执行的指令放到缓冲区中
void new_isa(word_t pc, word_t isa_val);
// cpu_exec 打印出最近执行的指令
void printIringBuf();

word_t expr(char *e, bool *success);

word_t paddr_read(paddr_t addr, int len);
void paddr_write(paddr_t addr, int len, word_t data);
#endif
