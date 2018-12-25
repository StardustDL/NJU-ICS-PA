#include "common.h"
#include <amdev.h>
#include <klib.h>

int vga_width = 0, vga_height = 0;

size_t serial_write(const void *buf, size_t offset, size_t len) {
  
  // _yield(); // Emulate slow device

  char *_buf = (char*) buf;
  size_t _len = len;
  while(_len--)
    _putc(*_buf++);
  return len;
}

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

size_t strnncpy(char* dst, const char* src, size_t n) {
  char *buf = strncpy(dst, src, n);
  size_t i = 0;
  for(; i<n && (*buf++) != '\0';i++);
  return i;
}

extern int f1_pcb, f2_pcb, f3_pcb;
void set_foreground(int id);

size_t events_read(void *buf, size_t offset, size_t len) {

  // _yield(); // Emulate slow device

  // Log("events_read %d %d", offset, len);
  static char eventinfo[128] __attribute__((used));
  // static int lastLen = 0;
  // if(offset>=lastLen){
    int key = read_key();
    if(key == _KEY_NONE){
      sprintf(eventinfo, "t %u\n", uptime());
    }
    else{
      if(key & 0x8000){
        int code = key ^ 0x8000;
        switch (code) {
          case _KEY_F1:
            set_foreground(f1_pcb);
            break;
          case _KEY_F2:
            set_foreground(f2_pcb);
            break;
          case _KEY_F3:
            set_foreground(f3_pcb);
            break;
        }
        sprintf(eventinfo, "kd %s\n", keyname[code]);
      }
      else{
        sprintf(eventinfo, "ku %s\n", keyname[key]);
      }
    }
    // lastLen = strlen(eventinfo);
  // }
  return strnncpy(buf, eventinfo /*+ offset*/, len);
}

static char dispinfo[128] __attribute__((used));

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  // Log("Dispinfo_read");
  return strnncpy(buf, dispinfo + offset, len);
}

static inline int min(int x, int y) {
  return (x < y) ? x : y;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {

  // _yield(); // Emulate slow device

  // printf("FB_write: offset: %d, len: %d\n", offset, len);
  int pos = offset / sizeof(uint32_t);
  int x = pos % vga_width;
  int y = pos / vga_width;
  int w = len / sizeof(uint32_t); // Warning!!! Not sure.
  int h = 1;
  uint32_t *pixels = (uint32_t*)buf;
  draw_rect(pixels, x, y, w, h);
  return len;
}

void init_device() {
  Log("Initializing devices...");
  _ioe_init();

  vga_width = screen_width();
  vga_height = screen_height();

  // printf("vga: width: %d, height: %d\n", vga_width, vga_height);

  sprintf(dispinfo, "WIDTH:%d\nHEIGHT:%d\n", vga_width, vga_height);

  // Log("%s",dispinfo);

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
}
