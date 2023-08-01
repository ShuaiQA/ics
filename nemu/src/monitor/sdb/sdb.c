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
#include "debug.h"
#include "macro.h"
#include "utils.h"
#include <cpu/cpu.h>
#include <isa.h>
#include <readline/history.h>
#include <readline/readline.h>

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin.
 */
static char *rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  nemu_state.state = NEMU_QUIT;
  return -1;
}

static int cmd_si(char *args) {
  char *token = strtok(args, " ");
  if (token == NULL) {
    cpu_exec(1);
  } else {
    int n = atoi(token);
    if (n == 0) {
      Log("输出10进制的整数");
    }
    cpu_exec(n);
  }
  return 0;
}

static int cmd_p(char *args) {
  if (args == NULL) {
    Log("p 需要参数,例如p $pc");
    return 0;
  }
  bool v;
  word_t pval = expr(args, &v);
  Log(FMT_WORD, pval);
  return 0;
}

// x 8 $mscratch
static int cmd_x(char *args) {
  char *token = strtok(args, " ");
  int cnt = -1;
  if (token != NULL) {
    sscanf(token, "%d", &cnt);
  }
  token = strtok(NULL, " "); // 表达式
  if (token == NULL) {
    Log("x 需要参数,例如x 4(10进制的数) 0x80000000, x 4 $pc");
  }
  bool v;
  word_t padd = expr(token, &v);
  for (int i = 0; i < cnt; i++) {
    if ((i % 4) == 0) {
      printf("[" FMT_WORD "]:\t", padd);
    }
    word_t pval = paddr_read(padd, 8);
    printf(FMT_WORD "\t ", pval);
    padd += 8;
    if ((i + 1) % 4 == 0) {
      printf("\n");
    }
  }
  printf("\n");
  return 0;
}

// 设置监视点
static int cmd_w(char *args) {
#ifdef CONFIG_WATCHPOINT
  if (args == NULL) {
    Log("w 应该加上参数,可以是变量或者表达式,内存地址,$寄存器名字");
    return 0;
  }
  bool success = false;
  word_t val = expr(args, &success);
  if (success == false) {
    Log("表达式不合法,重新输入\n");
  } else {
    new_wp(val, args);
    Log("当前的val是" FMT_WORD, val);
  }
#else
  Log("not define CONFIG_WATCHPOINT make menuconfig");
#endif
  return 0;
}

static int cmd_d(char *args) {
#ifdef CONFIG_WATCHPOINT
  if (args == NULL) {
    Log("d后面应该添加 info w 的NO\n");
  }
  int num = atoi(args);
  free_wp(num);
#else
  Log("not define CONFIG_WATCHPOINT make menuconfig");
#endif
  return 0;
}

// 查看所有的寄存器的值,或者是监视点
static int cmd_info(char *args) {
  char *token = strtok(args, " ");
  if (token == NULL) {
    Log("info参数不对输入:w,r,i,f,e,d\n");
    return 0;
  }
  switch (token[0]) {
  case 'r':
    isa_reg_display();
    break;
  case 'w':
    IFDEF(CONFIG_WATCHPOINT, printWP());
    break;
  case 'i':
    IFDEF(CONFIG_IRINGBUF, printIringBuf());
    break;
  case 'f':
    IFDEF(CONFIG_FTRACE, print_fun_buf());
    break;
  case 'e':
    IFDEF(CONFIG_ETRACE, print_etrace());
    break;
  case 'd':
    IFDEF(CONFIG_DTRACE, print_device_trace());
    break;
  default:
    Log("info参数不对输入:w,r,i,f,e,d\n");
  }
  return 0;
}

#define CONTEXT_BASE 0x8020d0000
#define CONTEXT_STACK 8*4096
static int cmd_pp(char *args){
  char *token = strtok(args, " ");
  word_t ctx;
  if (token != NULL) {
    sscanf(token, "%ld", &ctx);
  }
  // 获取当前的进程的cte指针的地址
  ctx = CONTEXT_BASE + ctx * CONTEXT_STACK;
  Log(FMT_WORD ,ctx );
  word_t ctx_addr = paddr_read(ctx , 8);
  Log("ctx addr is "FMT_WORD  ,ctx_addr);
  for(int i=0;i<36;i++){
    word_t reg = paddr_read(ctx_addr + i * 8, 8);
    printf(FMT_WORD  "  ",reg);
    if((i+1) %4 == 0){
      printf("\n");
    }
  }
  printf("\n");
  return 0;
}

static int cmd_help(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler)(char *);
} cmd_table[] = {
    {"help", "Display information about all supported commands", cmd_help},
    {"c", "Continue the execution of the program", cmd_c},
    {"q", "Exit NEMU", cmd_q},
    {"si", "step N", cmd_si},
    {"p", "print var", cmd_p},
    {"x", "look memory", cmd_x},
    {"w", "watchpoint", cmd_w},
    {"d", "删除监测点", cmd_d},
    {"pp","debug context",cmd_pp},
    {"info", "打印寄存器状态或打印监视点信息", cmd_info},

    /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  } else {
    for (i = 0; i < NR_CMD; i++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() { is_batch_mode = true; }

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL;) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) {
      continue;
    }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) {
          return;
        }
        break;
      }
    }

    if (i == NR_CMD) {
      printf("Unknown command '%s'\n", cmd);
    }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
