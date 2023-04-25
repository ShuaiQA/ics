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

#include "sdb.h"
#include "utils.h"

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  word_t value;  // 保存older_value
  char what[40]; // 保存监视的是什么名字
} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

// 从free_链表中返回一个空闲的监视点结构,添加到head
void new_wp(word_t value, char *what) {
  if (free_ == NULL) {
    assert(0);
  }
  WP *temp = free_;
  strcpy(temp->what, what);
  temp->value = value;
  free_ = free_->next;
  temp->next = head;
  head = temp;
}

// 将WP归还到free_链表中,删除查找head中的位置进行删除
void free_wp(int no) {
  if (head == NULL) {
    printf("观察点队列中没有no,输入info w查看所有的no");
    return;
  }
  WP *p = head; // 定位到wp的前一个节点
  if (p->NO == no) {
    head = head->next; // 删除head
    memset(p->what, '\0', 40);
    p->next = free_;
    free_ = p;
    return;
  }
  while (p->next != NULL && p->next->NO != no) {
    p = p->next;
  }
  if (p->next == NULL) {
    printf("观察点队列中没有no,输入info w查看所有的no");
    return;
  }
  WP *wp = p->next;
  p->next = wp->next;
  wp->next = free_;
  memset(wp->what, '\0', 40);
  free_ = wp;
}

void printWP() {
  WP *p = head;
  while (p != NULL) {
    printf("NO:[%d]   older value is [0x%08x]   表达式:[%s]\n", p->NO, p->value,
           p->what);
    p = p->next;
  }
}

void find_watch() {
  WP *p = head;
  bool v = false; // 判断但前是否含有监测点
  while (p != NULL) {
    // 查看当前的监测点是否合法,不需要在进行判断了因为插入的时候已经判断了
    bool temp = false;
    word_t val = expr(p->what, &temp);
    if (val != p->value) {
      printf("NO:[%d],表达式:[%s],older_value:[0x%08x],new_value:[0x%08x]\n",
             p->NO, p->what, p->value, val);
      p->value = val; // 更新val的值
      v = true;
    }
    p = p->next;
  }
  if (v) {
    nemu_state.state = NEMU_STOP;
  }
}
