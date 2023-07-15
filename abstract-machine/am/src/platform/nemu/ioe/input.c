#include <am.h>
#include <nemu.h>

#define KEYDOWN_MASK 0x8000

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  uint32_t f1 = inl(KBD_ADDR);
  kbd->keydown = f1 & KEYDOWN_MASK;
  kbd->keycode = f1 ^ KEYDOWN_MASK;
}
