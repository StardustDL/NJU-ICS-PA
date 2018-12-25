#include <am.h>
#include <amdev.h>

#include <ndl.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

static int W, H;

static inline int min(int x, int y) {
  return (x < y) ? x : y;
}

size_t video_read(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
    case _DEVREG_VIDEO_INFO: {
      _VideoInfoReg *info = (_VideoInfoReg *)buf;
      info->width = W;
      info->height = H;
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
      
      NDL_DrawRect(pixels, x, y, w, h);
      NDL_Render();
      // NDL_CloseDisplay();
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
  FILE *dispinfo = fopen("/proc/dispinfo", "r");
  assert(dispinfo);
  W = H = 0;
  char buf[128], key[128], value[128], *delim;
  while (fgets(buf, 128, dispinfo)) {
    *(delim = strchr(buf, ':')) = '\0';
    sscanf(buf, "%s", key);
    sscanf(delim + 1, "%s", value);
    if (strcmp(key, "WIDTH") == 0) sscanf(value, "%d", &W);
    if (strcmp(key, "HEIGHT") == 0) sscanf(value, "%d", &H);
  }
  fclose(dispinfo);
  assert(W > 0 && H > 0);


  NDL_OpenDisplay(W, H);
}
