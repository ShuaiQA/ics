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
#include <memory/paddr.h>

// 输出所有的断点集合   info w
void printWP();
// 创建一个断点         w $pc
void new_wp(word_t value, char *what);
// 释放一个断点         d 1
void free_wp(int no);
// 查看当前的值是否与断点值不一样    cpu_exec
bool find_watch();

// cpu_exec 将最近执行的指令放到缓冲区中
void new_isa(word_t pc, word_t isa_val);
// cpu_exec 打印出最近执行的指令
void printIringBuf();

// 查看当前的函数是否是属于函数集合范围
void new_fun(uint32_t pc);
// 输出函数调用路径
void print_fun_buf();

// 根据字符串e和设置字符串e是否符合规定success,判断返回值word_t
word_t expr(char *e, bool *success);

void set_elf_file(char *elf);

void new_device_trace(word_t pc, word_t addr, word_t data, const char *name);
void print_device_trace();

void new_etrace(word_t pc, word_t No);
void print_etrace();
#endif
