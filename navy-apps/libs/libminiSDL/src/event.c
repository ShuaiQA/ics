#include <NDL.h>
#include <SDL.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define keyname(k) #k,

static const char *keyname[] = {"NONE", _KEYS(keyname)};

#define LEN 10
static SDL_Event queue[LEN];
static int r = 0, w = 0;

int SDL_PushEvent(SDL_Event *ev) {
  queue[w] = *ev;
  w = (w + 1) % LEN;
  if (w == r) {
    printf("overflow");
    return 1;
  }
  return 0;
}

int SDL_PollEvent(SDL_Event *ev) {
  if (r != w) {
    *ev = queue[r];
    r = (r + 1) % LEN;
    return 1;
  }
  char buf[12];
  char down[1];
  char val[6];
  if (NDL_PollEvent(buf, sizeof(buf))) {
    sscanf(buf, "k%c %s\n", down, val);
    if (down[0] == 'd') {
      ev->type = SDL_KEYDOWN;
    } else {
      ev->type = SDL_KEYUP;
    }
    for (int i = 0; i < 85; i++) {
      if (strcmp(val, key[i]) == 0) {
        ev->key.keysym.sym = i + 1;
        break;
      }
    }
    return 1;
  }
  return 0;
}

int SDL_WaitEvent(SDL_Event *event) {
  while (!SDL_PollEvent(event))
    ;
  return 1;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  return 0;
}

uint8_t *SDL_GetKeyState(int *numkeys) { return NULL; }
