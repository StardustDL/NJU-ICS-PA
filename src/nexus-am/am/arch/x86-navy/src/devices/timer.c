#include <am.h>
#include <amdev.h>

#include <ndl.h>

uint32_t beginTime;

#define KEY_QUEUE_LEN 1024
static uint32_t key_queue[KEY_QUEUE_LEN];
static int key_f = 0, key_r = 0;

#define KEYDOWN_MASK 0x8000

uint32_t getKeyCode(){
  if (key_f != key_r) {
    uint32_t res = key_queue[key_f];
    key_f = (key_f + 1) % KEY_QUEUE_LEN;
    return res;
  }
  else {
    return _KEY_NONE;
  }
}

static uint32_t getTime(){
  NDL_Event e;
  do {
    NDL_WaitEvent(&e);
    if (e.type != NDL_EVENT_TIMER) {
      uint32_t am_scancode = e.data | ((e.type == NDL_EVENT_KEYDOWN) ? KEYDOWN_MASK : 0);
      key_queue[key_r] = am_scancode;
      key_r = (key_r + 1) % KEY_QUEUE_LEN;
    }
  } while (e.type != NDL_EVENT_TIMER);
  return e.data;
}

size_t timer_read(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
    case _DEVREG_TIMER_UPTIME: {
      _UptimeReg *uptime = (_UptimeReg *)buf;
      uptime->hi = 0;
      // uptime->lo = (clock() - beginTime) * 1000 / CLOCKS_PER_SEC;
      uptime->lo = (getTime() - beginTime);

      return sizeof(_UptimeReg);
    }
    case _DEVREG_TIMER_DATE: {
      _RTCReg *rtc = (_RTCReg *)buf;
      rtc->second = 0;
      rtc->minute = 0;
      rtc->hour   = 0;
      rtc->day    = 0;
      rtc->month  = 0;
      rtc->year   = 2018;
      return sizeof(_RTCReg);
    }
  }
  return 0;
}

void timer_init() {
  beginTime = getTime();
}
