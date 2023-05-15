#include <am.h>
#include <nemu.h>

#define KEYDOWN_MASK 0x8000

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  uint32_t f1 = inl(KBD_ADDR);
  int v = (f1 >> 15) & 0x1;
  kbd->keydown = (v == 0 ? 0 : 1);
  kbd->keycode = (v == 0 ? f1 : (f1 ^ KEYDOWN_MASK));
}
