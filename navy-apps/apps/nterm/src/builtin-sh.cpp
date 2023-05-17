#include <SDL.h>
#include <cstddef>
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

// 当前的cmd字符串是以\n进行结束的
static void sh_handle_cmd(const char *cmd) {
  char buf[100];
  size_t s = strlen(cmd);
  memcpy(buf, cmd, s - 1);

  char *argv[10];
  char *c = strtok(buf, " ");
  int i = 0;
  while (c != NULL) {
    argv[i] = c;
    i++;
    c = strtok(NULL, " ");
  }
  argv[i - 1] = NULL;
  for (int j = 0; j < i - 1; j++) {
    printf("j is %d string %s\n", j, argv[j]);
  }
  execve(argv[0], argv, NULL);
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
