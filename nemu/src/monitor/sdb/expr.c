/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include "common.h"
#include "debug.h"
#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>

enum {
  TK_NOTYPE = 256,
  TK_VAL = 255,

  /* TODO: Add more token types */
  TK_EQ = 0,
  TK_PLUS, TK_SUB, TK_MUL, TK_DIV,
  TK_DEREF, // 解引用，和乘法一样，但作为前缀
  TK_LBRACE, TK_RBRACE,
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

    /* TODO: Add more rules.
     * Pay attention to the precedence level of different rules.
     */

    {" +", TK_NOTYPE}, // spaces
                       // value
    {"(\\$)?([[:alnum:]]+)", TK_VAL},
    // (0[xX][0-9a-fA-F]+)|([1-9][0-9]*)|0, for integer
    {"\\+", TK_PLUS},   // plus
    {"==", TK_EQ},      // equal
    {"\\-", TK_SUB},    // sub
    {"\\*", TK_MUL},    // mul
    {"\\/", TK_DIV},    // div
                        // no definition for deref(*)
    {"\\(", TK_LBRACE}, // l_brace
    {"\\)", TK_RBRACE}, // r_brace
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
        case TK_NOTYPE:
          continue;
        case TK_VAL:
          tokens[nr_token].type = TK_VAL;
          if (substr_len < sizeof(tokens[0].str) / sizeof(char)) {
            printf("Value too long!\n");
            return false;
          }
          strncpy(tokens[nr_token].str, substr_start, substr_len);
          nr_token++;
          break;
        case TK_PLUS:
        case TK_SUB:
        case TK_MUL:
        case TK_DIV:
        case TK_EQ:
        case TK_LBRACE:
        case TK_RBRACE:
          tokens[nr_token++] = (Token){.type = rules[i].token_type, .str = ""};
          break;
        default:
          TODO();
        }
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}


word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  TODO();

  return 0;
}

bool check_parentheses(int p, int q) {
  if (tokens[p].type != TK_LBRACE || tokens[q].type != TK_RBRACE)
    return false;

  int idx = 0;

  for (int i = p; i <= q; ++i) {
    if (tokens[i].type == TK_LBRACE)
      idx++;
    else if (tokens[i].type == TK_RBRACE) {
      if (idx == 0)
        return false;
      idx--;
    }
  }

  return idx == 0;
}

word_t eval(int p, int q, bool *success) {
  if (p > q) {
    printf("Bad expression\n");
    *success = false;
    return -1;
  } else if (p == q) {
    Token token = tokens[p];
    Assert(token.type == TK_VAL, "Single token should be TK_VAL(%d), get %d\n",
           TK_VAL, token.type);
    char *str = token.str;
    word_t result = 0;
    if (strlen(str) > 2 && str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
      result = strtoul(str, NULL, 16);
    else if (str[0] == '0')
      result = strtoul(str, NULL, 10);
    else if (strlen(str) > 1 && str[0] == '$') {
      bool find = false;
      result = isa_reg_str2val(str + 1, &find);
      *success = find;
    } else {
      // 涉及到保存变量，需要用表来存储
      TODO();
    }
    return result;
  } else {
    if (tokens[p].type == TK_LBRACE && tokens[q].type == TK_RBRACE)
      return eval(p + 1, q - 1, success);
  }
  return 0;
}