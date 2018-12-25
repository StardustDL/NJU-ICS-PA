#include <x86.h>

#define PG_ALIGN __attribute((aligned(PGSIZE)))

static PDE kpdirs[NR_PDE] PG_ALIGN;
static PTE kptabs[PMEM_SIZE / PGSIZE] PG_ALIGN;
static void* (*pgalloc_usr)(size_t);
static void (*pgfree_usr)(void*);

_Area segments[] = {      // Kernel memory mappings
  {.start = (void*)0,          .end = (void*)PMEM_SIZE}
};

#define NR_KSEG_MAP (sizeof(segments) / sizeof(segments[0]))

int _vme_init(void* (*pgalloc_f)(size_t), void (*pgfree_f)(void*)) {
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  int i;

  // make all PDEs invalid
  for (i = 0; i < NR_PDE; i ++) {
    kpdirs[i] = 0;
  }

  PTE *ptab = kptabs;
  for (i = 0; i < NR_KSEG_MAP; i ++) {
    uint32_t pdir_idx = (uintptr_t)segments[i].start / (PGSIZE * NR_PTE);
    uint32_t pdir_idx_end = (uintptr_t)segments[i].end / (PGSIZE * NR_PTE);
    for (; pdir_idx < pdir_idx_end; pdir_idx ++) {
      // fill PDE
      kpdirs[pdir_idx] = (uintptr_t)ptab | PTE_P;

      // fill PTE
      PTE pte = PGADDR(pdir_idx, 0, 0) | PTE_P;
      PTE pte_end = PGADDR(pdir_idx + 1, 0, 0) | PTE_P;
      for (; pte < pte_end; pte += PGSIZE) {
        *ptab = pte;
        ptab ++;
      }
    }
  }

  set_cr3(kpdirs);
  set_cr0(get_cr0() | CR0_PG);

  return 0;
}

int _protect(_Protect *p) {
  PDE *updir = (PDE*)(pgalloc_usr(1));
  p->pgsize = 4096;
  p->ptr = updir;
  // map kernel space
  for (int i = 0; i < NR_PDE; i ++) {
    updir[i] = kpdirs[i];
  }

  p->area.start = (void*)0x8000000;
  p->area.end = (void*)0xc0000000;
  return 0;
}

void _unprotect(_Protect *p) {
}

static _Protect *cur_as = NULL;
void get_cur_as(_Context *c) {
  c->prot = cur_as;
}

void _switch(_Context *c) {
  set_cr3(c->prot->ptr);
  cur_as = c->prot;
}

#define ENABLE_MAP 0x1
#define DISABLE_MAP 0x0

int _map(_Protect *p, void *va, void *pa, int mode) {
  uint32_t vdir = PDX(va), vpage = PTX(va), pdir = PDX(pa), ppage = PTX(pa);
  PDE *p_dir = (PDE *) p->ptr;

  if((p_dir[vdir] & ENABLE_MAP) == 0) {
    p_dir[vdir] = (((uint32_t)pgalloc_usr(1)) | PTE_P); // Warning Not use PGADDR to build addr
  }
  PTE *p_tab = (PTE *) PTE_ADDR(p_dir[vdir]);
  if((mode & ENABLE_MAP) == 1) {
    p_tab[vpage] = PGADDR(pdir, ppage, 0) | PTE_P;
  }
  else{ // disable map
    p_tab[vpage] = PGADDR(pdir, ppage, 0) & (~PTE_P);
  }
  return 0;
}

#define EFLAG_ENABLE_IF 0x200

_Context *_ucontext(_Protect *p, _Area ustack, _Area kstack, void *entry, void *args) {
  // Create stack frame for crt0.c:_start
  uint32_t *stack_frame = (uint32_t *) ustack.end;
  stack_frame--; *stack_frame = (uint32_t) NULL; // envp
  stack_frame--; *stack_frame = (uint32_t) NULL; // argv
  stack_frame--; *stack_frame = (uint32_t) 0;    // argc
  stack_frame--; *stack_frame = (uint32_t) NULL; // return addr

  _Context *res = ((_Context *)(stack_frame - sizeof(_Context)));
  // memset(res, 0, sizeof(_Context));
  res->eip = (uintptr_t) entry;
  res->eflags = EFLAG_ENABLE_IF;
  res->cs = 8;
  res->prot = p;
  return res;
}
