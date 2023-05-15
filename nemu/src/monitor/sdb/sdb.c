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
    cpu_exec(n);
  }
  return 0;
}

// 查看所有的寄存器的值,或者是监视点
static int cmd_info(char *args) {
  char *token = strtok(args, " ");
  switch (token[0]) {
  case 'r':
    isa_reg_display();
    break;
  case 'w':
    printWP();
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
    Log("info参数不对输入:w或r\n");
  }
  return 0;
}

static int cmd_x(char *args) {
  char *token = strtok(args, " ");
  int cnt = -1;
  if (token != NULL) {
    sscanf(token, "%x", &cnt);
  }
  token = strtok(NULL, " "); // 表达式
  if (token == NULL) {
    Log("x 需要参数,例如x 4 0x80000000, x 4 $pc");
  }
  bool v;
  word_t padd = expr(token, &v);
  for (int i = 0; i < cnt; i++) {
    word_t pval = paddr_read(padd, 4);
    printf("[" FMT_WORD "]:\t" FMT_WORD "\t ", padd, pval);
    padd += 4;
    if ((i + 1) % 4 == 0) {
      printf("\n");
    }
  }
  printf("\n");
  return 0;
}

static int cmd_p(char *args) {
  if (args == NULL) {
    Log("p 需要参数,例如p $pc");
  }
  bool v;
  word_t pval = expr(args, &v);
  Log("" FMT_WORD, pval);
  return 0;
}

// 设置监视点
static int cmd_w(char *args) {
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
  return 0;
}

// 删除对应的监视点,不再进行监视
static int cmd_d(char *args) {
  if (args == NULL) {
    Log("d后面应该添加 info w 的NO\n");
  }
  int num = atoi(args);
  free_wp(num);
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
    {"si", "让程序单步执行N条指令后暂停执行, 当N没有给出时, 缺省为1", cmd_si},
    {"info", "打印寄存器状态或打印监视点信息", cmd_info},
    {"x", "扫描内存", cmd_x},
    {"p", "表达式求值", cmd_p},
    {"w", "监视点", cmd_w},
    {"d", "删除监测点", cmd_d},

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

    // 防止当前的调试输出到event_queue中
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
