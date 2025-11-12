#include "../src/monitor/sdb/expr.c"
#include "common.h"
#include <criterion/criterion.h>
#include <criterion/internal/assert.h>
#include <criterion/logging.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define LEN(list) sizeof(list) / sizeof(list[0])

Test(expr, check_parentheses) {
  // 测试用例
  Token *tk = tokens;
  int tk_pqr[][3] = {
      {0, 1, true},  {0, 2, true},  {0, 4, true},  {0, 3, false},
      {0, 8, false}, {0, 0, false}, {0, 1, false}, {0, 1, false},
      {0, 1, false}, {0, 2, false}, {0, 2, false},
  };
  int tk_list[][32] = {
      {-1, 1},
      {-1, 0, 1},
      {-1, 0, 0, 0, 1},
      {-1, 1, -1, 1},
      {-1, 0, 1, -1, 0, 1, -1, 0, 1},
      {-1},
      {1},
      {1, -1},
      {-1, -1, 1, 0},
      {-1, 1, 1},
  };

  // Test begin
  int *tks, p, q, r;
  for (int i = 0; i < LEN(tk_list); ++i) {
    tks = tk_list[i];
    for (int j = 0; j < 32; ++j) {
      if (tks[j] == -1)
        tk[j].type = TK_LBRACE;
      else if (tks[j] == 1)
        tk[j].type = TK_RBRACE;
      else
        tk[j].type = TK_NOTYPE;
    }
    p = tk_pqr[i][0], q = tk_pqr[i][1], r = tk_pqr[i][2];
    cr_expect_eq(check_parentheses(p, q), r, "TEST CASE %d failed", i);
  }
}

Test(expr, eval) {
  // 环境准备
  init_regex();

  // 测试用例
  char *test_cases[] = {
      "1 + 2 + 3",       "(1 + 2)",     "1 - 2",      "1 + 2 * 3",
      "2 * 3 + 4",       "(2 + 3) * 4", "10 / 2",     "11 / 2",
      "4 + (6 - 3) * 2", "1 + 2 == 3",  "2 * 2 == 5", "(1 + (2 * 3)) == 7",
      "1 + -1",          "--1",
  };
  word_t results[] = {6, 3, -1, 7, 10, 20, 5, 5, 10, 1, 0, 1, 0, 1};

  for (int i = 0; i < LEN(test_cases); ++i) {
    bool success;
    word_t result = expr(test_cases[i], &success);
    cr_expect_eq(result, results[i],
                 "TEST CASE %d failed, result: %d, need: %d", i, result,
                 results[i]);
  }
}

word_t make_expr(int depth, char *expr, int len) {
  word_t result = 0;
  if (depth == 0) {
    result = rand() % (1 << 5);
    snprintf(expr, 255, "%d", result);
    return result;
  }

  int choice = rand() % 7;
  char l_expr[256] = {0}, r_expr[256] = {0};
  switch (choice) {
  case 0: // ? == ?
    result = make_expr(depth - 1, l_expr, sizeof(l_expr)) ==
             make_expr(depth - 1, r_expr, sizeof(r_expr));
    snprintf(expr, len, "(%s)==(%s)", l_expr, r_expr);
    break;
  case 1: // ? + ?
    result = make_expr(depth - 1, l_expr, sizeof(l_expr)) +
             make_expr(depth - 1, r_expr, sizeof(r_expr));
    snprintf(expr, len, "(%s)+(%s)", l_expr, r_expr);
    break;
  case 2: // ? - ?
    result = make_expr(depth - 1, l_expr, sizeof(l_expr)) -
             make_expr(depth - 1, r_expr, sizeof(r_expr));
    snprintf(expr, len, "(%s)-(%s)", l_expr, r_expr);
    break;
  case 3: // ? * ?
    result = make_expr(depth - 1, l_expr, sizeof(l_expr)) *
             make_expr(depth - 1, r_expr, sizeof(r_expr));
    snprintf(expr, len, "(%s)*(%s)", l_expr, r_expr);
    break;
  case 4: // ? / ?
    result = make_expr(depth - 1, l_expr, sizeof(l_expr));
    word_t tmp = make_expr(depth - 1, r_expr, sizeof(r_expr));
    while ((tmp = make_expr(depth - 1, r_expr, sizeof(r_expr))) == 0);
    result /= tmp;
    snprintf(expr, len, "(%s)/(%s)", l_expr, r_expr);
    break;
  case 5: // -?
    result = -make_expr(depth - 1, l_expr, sizeof(l_expr));
    snprintf(expr, len, "-(%s)", l_expr);
    break;
  case 6: // (?)
    result = make_expr(depth - 1, l_expr, sizeof(l_expr));
    snprintf(expr, len, "(%s)", l_expr);
    break;
  default:
    break;
  }

  return result;
}

Test(expr, eval_random) {
  Log("Test expr: eval_random");
  init_regex();
  srand(time(NULL));

  const int total = 10, max_depth = 3;
  char e[256];
  bool success;
  word_t value, result;
  for (int i = 0; i < total; ++i) {
    memset(e, 0, sizeof(e));
    value = make_expr(rand() % max_depth, e, sizeof(e));
    Log("expr: %s, value: %d", e, value);
    result = expr(e, &success);
    cr_expect_eq(value, result, "Test case %d failed\n", i);
  }
}