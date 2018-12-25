#include <am.h>
#include <amdev.h>

#include <ndl.h>

uint32_t getKeyCode(); // at timer.c

#define KEYDOWN_MASK 0x8000

size_t input_read(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
    case _DEVREG_INPUT_KBD: {
      _KbdReg *kbd = (_KbdReg *)buf;
      // uint32_t data = getchar();
      // kbd->keycode = data;
      // kbd->keydown = (data & KEYDOWN_MASK) > 0;
      // kbd->keydown = 0;

/*
      NDL_Event e;
      int cnt = 0;
      do {
        NDL_WaitEvent(&e);
        // cnt++;
      } while (cnt < 10 && e.type == NDL_EVENT_TIMER);
      if (e.type == NDL_EVENT_TIMER){
        kbd->keycode = 0;
        kbd->keydown = 0;
      }
      else {
        kbd->keycode = e.data;
        kbd->keydown = e.type == NDL_EVENT_KEYDOWN;
      }*/

      
      uint32_t data = getKeyCode();
      kbd->keycode = data & (~KEYDOWN_MASK);
      kbd->keydown = (data & KEYDOWN_MASK) > 0;
      
      return sizeof(_KbdReg);
    }
  }
  return 0;
}
