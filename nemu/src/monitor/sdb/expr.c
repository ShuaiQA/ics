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

#include <isa.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, // " "
  TK_ZUO,          // "("
  TK_YOU,          // ")"
  // 逻辑运算
  TK_EQ,   // "=="
  TK_NEQ,  // "!="
  TK_YUYU, // "&&"
  // 算术运算
  TK_ADD, // "+"   262
  TK_SUB, // "-"
  TK_MUL, // "*"
  TK_DIV, // "/"
  // 添加变量
  TK_NUM,   // "[0-9]+"  266
  TK_0XNUM, // "0x"
  TK_REG,   // "$"
  // 引用
  TK_DEREF, // "*"
  /* TODO: Add more token types */
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {
    {" ", TK_NOTYPE},
    {"0x([0-9a-f]+)", TK_0XNUM},
    {"\\$[0-9a-f]+", TK_REG},
    {"&&", TK_YUYU},
    {"==", TK_EQ},
    {"!=", TK_NEQ},
    {"\\+", TK_ADD},
    {"-", TK_SUB},
    {"\\*", TK_MUL},
    {"/", TK_DIV},
    {"\\(", TK_ZUO},
    {"\\)", TK_YOU},
    {"([0-9]+)", TK_NUM}, // 0-9数字
};

#define NR_REGEX ARRLEN(rules)
word_t paddr_read(paddr_t addr, int len);

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i++) {
    // 进行初始化re数组用来正则表达式匹配
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;     // 标记类型例如+-*/
  char str[32]; // 记录数字的大小
} Token;

static Token tokens[32]
    __attribute__((used)) = {}; // 按照顺序存放已经被识别的token数组
static int nr_token __attribute__((used)) = 0; // 已经被识别的token的数目

static bool make_token(char *e) {
  int position = 0; // 表示当前处理到e表达式的位置
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i++) {
      // 在字符串e中查找re[i]字符串,将查找到的匹配的e中起始位置和结束位置标记放在结构体变量pmatch中
      // 有时可能会在字符串中查找多个匹配的起始和结束,但是本例子只查找当前匹配一次
      // 变量1代表pmatch记录的大小
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 &&
          pmatch.rm_so == 0) { // 找到匹配的字符串并且匹配的位置应该从0开始
                               // 找到字符串开始的位置和长度
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;
        /* printf("match rules[%d] = \"%s\" at position %d with len %d: %.*s\n",
           i, rules[i].regex, position, substr_len, substr_len, substr_start);
         */
        position += substr_len;
        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         * 现在已经识别了一个新的token在rules[i]中,现在将其添加到tokens中
         * 对于一些token还需要添加额外的信息
         */
        switch (rules[i].token_type) {
        case TK_ADD: //"+"
          tokens[nr_token++].type = TK_ADD;
          break;
        case TK_EQ: //"=="
          tokens[nr_token++].type = TK_EQ;
          break;
        case TK_NEQ: //"!="
          tokens[nr_token++].type = TK_NEQ;
          break;
        case TK_DIV: //"/"
          tokens[nr_token++].type = TK_DIV;
          break;
        case TK_MUL: //"*"
          tokens[nr_token++].type = TK_MUL;
          break;
        case TK_SUB: //"-"
          tokens[nr_token++].type = TK_SUB;
          break;
        case TK_YUYU:
          tokens[nr_token++].type = TK_YUYU;
          break;
        case TK_NOTYPE: // 空格直接删除
          break;
        case TK_ZUO: //"("
          tokens[nr_token++].type = TK_ZUO;
          break;
        case TK_YOU: //")"
          tokens[nr_token++].type = TK_YOU;
          break;
        case TK_REG:
          tokens[nr_token].type = TK_REG;
          strncpy(tokens[nr_token].str, substr_start + 1, substr_len - 1);
          tokens[nr_token].str[substr_len - 1] = '\0';
          nr_token++;
          break;
        case TK_0XNUM:
          tokens[nr_token].type = TK_0XNUM;
          strncpy(tokens[nr_token].str, substr_start, substr_len);
          tokens[nr_token].str[substr_len] = '\0';
          nr_token++;
          break;
        default: // 变量名
          tokens[nr_token].type = TK_NUM;
          strncpy(tokens[nr_token].str, substr_start, substr_len);
          tokens[nr_token].str[substr_len] = '\0';
          nr_token++;
          break;
        }
        break;
      }
    }
    // 在匹配token中没有找到与当前匹配的token输出没有找到
    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

// 检查当前的 [l,r]是否符合括号匹配
bool check_parentheses(int l, int r) {
  int ll = 0, rr = 0;
  for (int i = l; i <= r; i++) {
    if (tokens[i].type == TK_ZUO) {
      ll++;
    } else if (tokens[i].type == TK_YOU) {
      rr++;
    }
    if (ll < rr) {
      return false;
    }
  }
  return ll == rr;
}

int find_main(int l, int r) {
  int ll = 0;                    // 标记当前的位置是否处在括号内
  int you[4] = {-1, -1, -1, -1}; // 记录每个优先级对应的下标
  for (int i = l; i <= r; i++) {
    if (tokens[i].type == TK_ZUO) {
      ll++;
      continue;
    } else if (tokens[i].type == TK_YOU) {
      ll--;
      continue;
    }
    // 当前处在括号内,并且当前是0x $reg var直接跳过
    if (ll > 0 || tokens[i].type == TK_0XNUM || tokens[i].type == TK_REG ||
        tokens[i].type == TK_NUM) {
      continue;
    }
    if (tokens[i].type == TK_MUL || tokens[i].type == TK_DIV) {
      you[0] = i;
    }
    if (tokens[i].type == TK_ADD || tokens[i].type == TK_SUB) {
      you[1] = i;
    }
    if (tokens[i].type == TK_EQ || tokens[i].type == TK_NEQ) {
      you[2] = i;
    }
    if (tokens[i].type == TK_YUYU) {
      you[3] = i;
    }
  }
  int pos = -1;
  for (int i = 3; i >= 0; i--) {
    if (you[i] >= 0) {
      pos = you[i];
      break;
    }
  }
  assert(pos != -1);
  return pos;
}

bool yinyong(int l, int r) {
  for (int i = l; i < r; i++) {
    if (tokens[i].type != TK_DEREF) {
      return false;
    }
  }
  if (tokens[r].type == TK_REG || tokens[r].type == TK_NUM ||
      tokens[r].type == TK_0XNUM) {
    return true;
  } else {
    return false;
  }
}

word_t eval(int l, int r) {
  assert(l <= r);
  if (l == r) {
    // 是变量、寄存器、16进制的数
    if (tokens[l].type == TK_NUM) {
      return atoi(tokens[l].str);
    } else if (tokens[l].type == TK_REG) {
      bool vis = false;
      return isa_reg_str2val(tokens[l].str, &vis);
    } else { // TK_0XNUM
      word_t cur = -1;
      sscanf(tokens[l].str, "%x", &cur);
      return cur;
    }
  } else if (tokens[l].type == TK_ZUO && tokens[r].type == TK_YOU &&
             check_parentheses(l + 1, r - 1) == true) {
    return eval(l + 1, r - 1);
  } else if (yinyong(l, r)) {
    // 根据分析可以得到引用是最高优先级的,最后必定留下两个,又根据我们是相同的符号左边的优先级较高
    word_t val = eval(r, r);
    for (int i = 0; i < r - l; i++) {
      val = paddr_read(val, 4);
    }
    return val;
  } else { // 查找主运算符(最后一步运算),首先排除括号里面的运算符,然后根据算术优先进行选择
    int pos = find_main(l, r);
    word_t val1 = eval(l, pos - 1);
    word_t val2 = eval(pos + 1, r);
    switch (tokens[pos].type) {
      // 溢出判断
    case TK_ADD:
      return val1 + val2;
    case TK_SUB:
      return val1 - val2;
    case TK_MUL:
      return val1 * val2;
    case TK_DIV:
      assert(val2 != 0);
      return val1 / val2;
    case TK_EQ:
      return val1 == val2;
    case TK_NEQ:
      return val1 != val2;
    case TK_YUYU:
      return val1 && val2;
    default:
      assert(0);
    }
  }
  return 0;
}

/*
 * 对于表达式求值我们只考虑三种情况的组合
 * 1、w 0x80000000   监视内存位置0x80000000
 * 2、w $pc          监视寄存器pc的值
 * 3、w 表达式       监视变量或者表达式的值
 * 所有的数据都是32位的,无论是指针解引用32位,类型什么的
 */
word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  for (int i = 0; i < nr_token; i++) {
    if (tokens[i].type == TK_MUL &&
        (i == 0 || tokens[i - 1].type == TK_ADD ||
         tokens[i - 1].type == TK_SUB || tokens[i - 1].type == TK_MUL ||
         tokens[i - 1].type == TK_DIV || tokens[i - 1].type == TK_EQ ||
         tokens[i - 1].type == TK_NEQ || tokens[i - 1].type == TK_YUYU ||
         tokens[i - 1].type == TK_DEREF)) { // + - * / == != && *
      tokens[i].type = TK_DEREF;
    }
  }
  // 根据表达式tokens以及nr_token计算表达式
  assert(check_parentheses(0, nr_token - 1) == true); // 检查当前的括号是否匹配
  word_t val = eval(0, nr_token - 1);
  *success = true;
  return val;
}
