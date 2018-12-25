#include <am.h>
#include <x86.h>
#include <amdev.h>
#include <klib.h>

#define VMEM 0x40000
#define SCREEN_PORT 0x100 // Note that this is not the standard

static uint32_t* const fb __attribute__((used)) = (uint32_t *)0x40000;

static int W, H;

static inline int min(int x, int y) {
  return (x < y) ? x : y;
}

size_t video_read(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
    case _DEVREG_VIDEO_INFO: {
      _VideoInfoReg *info = (_VideoInfoReg *)buf;
      uint32_t data = inl(SCREEN_PORT);
      info->width = data >> 16;
      info->height = data & 0xffff;
      return sizeof(_VideoInfoReg);
    }
  }
  return 0;
}

size_t video_write(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
    case _DEVREG_VIDEO_FBCTL: {
      _FBCtlReg *ctl = (_FBCtlReg *)buf;

      /* Test
      int i;
      int size = screen_width() * screen_height();
      for (i = 0; i < size; i ++) fb[i] = i;*/

      int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
      uint32_t *pixels = ctl->pixels;
      // int cp_bytes = sizeof(uint32_t) * min(w, W - x);
      int cp_uints = min(w, W - x);
      for (int j = 0; j < h && y + j < H; j ++) {
        // memcpy(&fb[(y + j) * W + x], pixels, cp_bytes);
        for(int i = 0; i < cp_uints; i++)
          fb[(y+j)*W+x+i] = pixels[i];
        pixels += w;
      }

      if (ctl->sync) {
        // do nothing, hardware syncs.
      }
      /*for(int i = 0; i < ctl->w; i++){
        if(ctl->x + i >= W) break;
        int cbg = bg + i * W;
        for(int j = 0; j < ctl->h; j++){
          if(ctl->y + j >= H) break;
          fb[cbg + j] = ctl->pixels[i * ctl->w + j];
        }
      }*/

      return sizeof(_FBCtlReg);
    }
  }
  return 0;
}

void vga_init() {
  W = screen_width(), H = screen_height();
  memset(fb, 0, W * H * sizeof(uint32_t));
}
