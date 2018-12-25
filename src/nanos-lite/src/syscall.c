#include "common.h"
#include "syscall.h"
#include "fs.h"
#include "proc.h"

void naive_uload(PCB *pcb, const char *filename);
void context_uload(PCB *pcb, const char *filename);
int mm_brk(uintptr_t new_brk);

int sys_yield() {
  _yield();
  return 0;
}

int sys_execve(const char *filename, char *const argv[], char *const envp[]){
  // naive_uload(NULL, filename);
  // context_uload(current, filename);

  new_proc(filename, 100);

  exit_proc(); // exit /bin/init

  return 0;
}

void sys_exit(int code) {
  // Log("Halt for dummy");
  // _halt(code);
  Log("Exited with %d", code);
  
  new_proc("/bin/init", 10);

  exit_proc();
  
  return;
  // sys_execve("/bin/init", NULL, NULL);
  _halt(code);
}

/*int sys_write(int fd, const void *buf, size_t count){
  // printf("fd: %d, buf: %s, count: %d\n",fd,buf,count);
  if (fd == 1 || fd == 2) { // for stdout stderr
    char* b = (char*) buf;
    int cnt = count;
    while(cnt--)
      _putc(*b++);
    return count; // The number output successfully 
  }
  else return -1; // Error
}*/

int sys_brk(void * addr){
  return mm_brk((uintptr_t) addr);

  // OS set heap point
  /*static void * program_break = 0;
  program_break = addr;*/
  // return 0; // Always success
}

int sys_open(const char *pathname, int flags, int mode){
  return fs_open(pathname, flags, mode);
}

ssize_t sys_read(int fd, void *buf, size_t len){
  return fs_read(fd, buf, len);
}

ssize_t sys_write(int fd, const void *buf, size_t len){
  return fs_write(fd, buf, len);
}

off_t sys_lseek(int fd, off_t offset, int whence){
  return fs_lseek(fd, offset, whence);
}

int sys_close(int fd){
  return fs_close(fd);
}

/*
typedef long clock_t;

struct tms {
    clock_t tms_utime;
    clock_t tms_stime;
    clock_t tms_cutime;
    clock_t tms_cstime;
};

clock_t sys_times(struct tms* tbuf){
  clock_t clk = (clock_t) uptime();
  tbuf->tms_utime = clk;
  tbuf->tms_stime = 0;
  tbuf->tms_cutime = 0;
  tbuf->tms_cstime = 0;
  return clk;
}
*/

_Context* do_syscall(_Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;

  // Log("%s: %d", "system call",a[0]);

  switch (a[0]) {
    case SYS_yield:
      c->GPRx = sys_yield();
      break;
    case SYS_exit:
      sys_exit(a[1]);
      break;
    case SYS_brk:
      c->GPRx = sys_brk((void*)a[1]);
      break;
    case SYS_open:
      c->GPRx = sys_open((const char*) (a[1]), a[2], a[3]);
      break;
    case SYS_read:
      c->GPRx = sys_read(a[1], (void *) a[2], (size_t) a[3]);
      break;
    case SYS_write:
      c->GPRx = sys_write(a[1], (const void *) a[2], (size_t) a[3]);
      break;
    case SYS_lseek:
      c->GPRx = sys_lseek(a[1], a[2], a[3]);
      break;
    case SYS_close:
      c->GPRx = sys_close(a[1]);
      break;
    case SYS_execve:
      c->GPRx = sys_execve((const char *) a[1], (char *const *) a[2], (char *const *) a[3]);
      break;
    // case SYS_times:
      // c->GPRx = sys_times((struct tms *) a[1]);
      // break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
  // Log("%s: %d", "system called",a[0]);
  return NULL;
}
