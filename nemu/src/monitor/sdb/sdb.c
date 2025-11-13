/***************************************************************************************
 * Copyright (c) 2014-2024 Zihao Yu, Nanjing University
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
#include <cpu/cpu.h>
#include <isa.h>
#include <memory/vaddr.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <stdint.h>

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();
void info_watchpoints();

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

static int cmd_help(char *args);
static int cmd_c(char *args);
static int cmd_q(char *args);
static int cmd_si(char *args);
static int cmd_info(char *args);
static int cmd_x(char *args);
static int cmd_p(char *args);
static int cmd_w(char *args);
static int cmd_d(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler)(char *);
} cmd_table[] = {
    {"help", "Display information about all supported commands", cmd_help},
    {"c", "Continue the execution of the program", cmd_c},
    {"q", "Exit NEMU", cmd_q},

    /* TODO: Add more commands */
    {"si", "Step i steps", cmd_si},
    {"info", "Display information about the process", cmd_info},
    {"x", "Scan N words from address EXPR", cmd_x},
    {"p", "Calculate value for EXPR", cmd_p},
    {"w", "Set a watchpoint with EXPR", cmd_w},
    {"d", "Remove watchpoint with ID N", cmd_d},
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

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) { return -1; }

static int cmd_si(char *args) {
  args = strtok(args, " ");
  int steps = 1;
  if (args != NULL)
    steps = atoi(args);
  cpu_exec(steps);
  return 0;
}

static int cmd_info(char *args) {
  if (strcmp(args, "r") == 0) {
    isa_reg_display();
  } else if (strcmp(args, "w") == 0) {
    info_watchpoints();
  }
  return 0;
}

static int cmd_x(char *args) {
  args = strtok(args, " ");
  if (args == NULL) {
    printf("[ERROR]: no N input\n");
  }
  int n = atoi(args);
  args = strtok(NULL, " ");
  // TODO：目前的逻辑输入为纯16进制数
  if (args == NULL) {
    printf("[ERROR]: no EXPR input\n");
  }
  bool success;
  uint32_t addr = expr(args, &success);

  if (!success) {
    printf("Invalid EXPR\n");
    return 1;
  }

  for (int i = 0; i < n; ++i) {
    printf("0x%x", vaddr_read(addr, 4));
    if (i % 4 == 3)
      printf("\n");
    else
      printf("\t");
    addr += 4;
  }

  return 0;
}

static int cmd_p(char *args) {
  bool success;
  word_t result = expr(args, &success);
  if (!success) {
    printf("Invalid expr!\n");
    return 1;
  }
  printf("%d\n", result);
  return 0;
}

WP *wps[NR_WP] = {};
static int cmd_w(char *args) {
  WP *wp = new_wp();
  wp->str = args;
  wps[wp->NO] = wp;
  return 0;
}

static int cmd_d(char *args) {
  int n = atoi(args);
  if (wps[n] == NULL) {
    printf("No watchpoint with NO %d\n", n);
    return 1;
  }
  free_wp(wps[n]);
  wps[n] = NULL;
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
