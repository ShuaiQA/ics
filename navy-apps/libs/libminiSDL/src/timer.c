#include <NDL.h>
#include <sdl-timer.h>
#include <stdint.h>
#include <stdio.h>

// 当时间interval过去之后调用一个回调函数callback
SDL_TimerID SDL_AddTimer(uint32_t interval, SDL_NewTimerCallback callback,
                         void *param) {
  return NULL;
}

int SDL_RemoveTimer(SDL_TimerID id) { return 1; }

// 返回32位的毫秒值
uint32_t SDL_GetTicks() { return NDL_GetTicks() / 1000; }

void SDL_Delay(uint32_t ms) {
  uint32_t cur = SDL_GetTicks();
  while (1) {
    uint32_t next = SDL_GetTicks();
    if (next - cur > ms) {
      break;
    }
  }
}
