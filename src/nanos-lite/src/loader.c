#include "proc.h"
#include "memory.h"
#include "fs.h"

// #define DEFAULT_ENTRY 0x4000000
#define DEFAULT_ENTRY 0x8048000

int _protect(_Protect *p);

static uintptr_t loader(PCB *pcb, const char *filename) {

  int fd = fs_open(filename, 0, 0);
  size_t fsize = fs_filesz(fd);

#ifdef HAS_VME

  size_t fpos = 0, vaddr_pos = DEFAULT_ENTRY;
  while(fpos < fsize){
    void* pg = new_page(1);
    _map(&pcb->as, (void *) vaddr_pos, pg, ENABLE_MAP);
    size_t readed = fs_read(fd, pg, PGSIZE);
    fpos += readed;
    vaddr_pos += readed;
  }

  pcb->max_brk = pcb->cur_brk = ((vaddr_pos >> 12) + 1) << 12; // use next page for brk

#else
  void *buf = (void*) DEFAULT_ENTRY;

  // ramdisk_read(buf, 0, get_ramdisk_size());

  fs_read(fd, buf, fsize);
#endif

  Log("Loaded %s %x", filename, DEFAULT_ENTRY);

  return DEFAULT_ENTRY;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  ((void(*)())entry) ();
}

void context_kload(PCB *pcb, void *entry) {
  _Area stack;
  stack.start = pcb->stack;
  stack.end = stack.start + sizeof(pcb->stack);

  pcb->cp = _kcontext(stack, entry, NULL);
}

void context_uload(PCB *pcb, const char *filename) {
  _protect(&pcb->as);
  uintptr_t entry = loader(pcb, filename);

  _Area stack;
  stack.start = pcb->stack;
  stack.end = stack.start + sizeof(pcb->stack);

  pcb->cp = _ucontext(&pcb->as, stack, stack, (void *)entry, NULL);
}
