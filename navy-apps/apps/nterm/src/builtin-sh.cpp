#include <SDL.h>
#include <cstddef>
#include <cstdio>
#include <nterm.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

char handle_key(SDL_Event *ev);

static void sh_printf(const char *format, ...) {
  static char buf[256] = {};
  va_list ap;
  va_start(ap, format);
  int len = vsnprintf(buf, 256, format, ap);
  va_end(ap);
  term->write(buf, len);
}

static void sh_banner() {
  sh_printf("Built-in Shell in NTerm (NJU Terminal)\n\n");
}

static void sh_prompt() { sh_printf("sh> "); }

static void sh_handle_cmd(const char *cmd) {
  static char buf[256];
  memmove(buf, cmd, strlen(cmd));
  char *pathname = strtok(buf, " ");
  // 只有参数使用最后使用\n进行结尾
  char *argv[10]; // 最大的参数是10
  argv[0] = NULL;
  char *para = strtok(NULL, " ");
  int i = 0;
  for (; para != NULL; i++) {
    argv[i] = para;
    para = strtok(NULL, " ");
  }
  // 删除\n变为\0
  if (i == 0) {
    pathname[strlen(pathname) - 1] = '\0';
  } else {
    argv[i - 1][strlen(argv[i - 1]) - 1] = '\0';
    argv[i] = NULL;
  }
  // printf("[%s] \n", pathname);
  // for (int i = 0; argv[i] != NULL; i++) {
  //   printf("argv [%d] %s\n", i, argv[i]);
  // }
  execve(pathname, argv, NULL);
}

void builtin_sh_run() {
  sh_banner();
  sh_prompt();

  while (1) {
    SDL_Event ev;
    if (SDL_PollEvent(&ev)) {
      if (ev.type == SDL_KEYUP || ev.type == SDL_KEYDOWN) {
        const char *res = term->keypress(handle_key(&ev));
        if (res) {
          sh_handle_cmd(res);
          sh_prompt();
        }
      }
    }
    refresh_terminal();
  }
}
