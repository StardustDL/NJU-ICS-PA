#include "fs.h"

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

int strcmp(const char *s1, const char *s2);
size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);
size_t get_ramdisk_size();

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  size_t open_offset;
  ReadFn read;
  WriteFn write;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

size_t serial_write(const void *buf, size_t offset, size_t len);
size_t fb_write(const void *buf, size_t offset, size_t len);
size_t dispinfo_read(void *buf, size_t offset, size_t len);
size_t events_read(void *buf, size_t offset, size_t len);

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin", 0, 0, 0, invalid_read, invalid_write},
  {"stdout", 0, 0, 0, invalid_read, serial_write},
  {"stderr", 0, 0, 0, invalid_read, serial_write},
  {"/dev/fb", 0, 0, 0, invalid_read, fb_write},
  {"/proc/dispinfo", 0, 0, 0, dispinfo_read, invalid_write},
  {"/dev/events", 0, 0, 0, events_read, invalid_write},
  {"/dev/tty",0, 0, 0, invalid_read, serial_write},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

int vga_width , vga_height; // defined at device.c

void init_fs() {
  // TODO: initialize the size of /dev/fb
  // printf("!!!\n");
  Finfo *fb = &file_table[FD_FB];
  fb->size = vga_width * vga_height * sizeof(uint32_t);
  // printf("!!!! %d\n",fb->size);
}

int fs_open(const char *pathname, int flags, int mode){
  for(int fd = 0; fd < NR_FILES; fd++){
    if(strcmp(pathname, file_table[fd].name) == 0){
      // assert(file_table[fd].open_offset == 0);
      file_table[fd].open_offset = 0;
      // Log("open file: %s",pathname);
      return fd;
    }
  }
  panic("file open failed: %s",pathname);
}

ssize_t fs_read(int fd, void *buf, size_t len){
  assert(0 <= fd && fd < NR_FILES);

  if(fd == FD_STDIN || fd == FD_STDOUT || fd == FD_STDERR) // ignore stdio
    return 0;

  Finfo* f = &file_table[fd];

  size_t real_len = len;

  if(f->read != NULL){
    real_len = f->read(buf, f->open_offset, len);
  }
  else{
    if (f->open_offset + len > f->size)
      real_len = f->size - f->open_offset;
    
    ramdisk_read(buf, f->disk_offset + f->open_offset, real_len);
  }

  f->open_offset += real_len;
  return real_len;
}

ssize_t fs_write(int fd, const void *buf, size_t len){
  assert(0 <= fd && fd < NR_FILES);

  if(fd == FD_STDIN) // ignore stdin
    return 0;

  Finfo* f = &file_table[fd];

  size_t real_len = len;

  if(f->write != NULL){
    real_len = f->write(buf, f->open_offset, len);
  }
  else{
    if (f->open_offset + len > f->size)
      real_len = f->size - f->open_offset;
  
    ramdisk_write(buf, f->disk_offset + f->open_offset, real_len);
  }
  
  f->open_offset += real_len;

  return real_len;
}

off_t fs_lseek(int fd, off_t offset, int whence){
  assert(0 <= fd && fd < NR_FILES);

  if(fd == FD_STDIN || fd == FD_STDOUT || fd == FD_STDERR) // ignore stdio
    return 0;

  Finfo* f = &file_table[fd];
  switch(whence){
    case SEEK_SET:
      {
        if(offset <= f->size)
          f->open_offset = offset;
        else return (off_t)-1;
      }
      break;
    case SEEK_CUR:
      {
        if(f->open_offset + offset <= f->size)
          f->open_offset += offset;
        else return (off_t)-1;
      }
      break;
    case SEEK_END: // ???
      {
        if(f->size + offset <= f->size)
          f->open_offset = f->size + offset; 
        else return (off_t)-1;
      }
      break;
    default:
      panic("file lseek with wrong whence");
      break;
  }
  return (off_t) f->open_offset;
}

int fs_close(int fd){
  assert(0 <= fd && fd < NR_FILES);
  return 0; // Always success
}

size_t fs_filesz(int fd){
  assert(0 <= fd && fd < NR_FILES);
  Finfo* f = &file_table[fd];
  return f->size;
}