#include <am.h>
#include <x86.h>

static _Context* (*user_handler)(_Event, _Context*) = NULL;

int printf(const char* fmt, ...);

void vectrap();
void vecsys();
void vecnull();
void irq0();

#define IRQ_vectrap 0x81
#define IRQ_vecsys 0x80
#define IRQ_TIMER 32

void get_cur_as(_Context *c);

void _switch(_Context *c);

_Context* irq_handle(_Context *tf) {
  get_cur_as(tf);
  _Context *next = tf;

  /*
  // Check _Context
  printf("eax: %u\n", tf->eax);
  printf("ebx: %u\n", tf->ebx);
  printf("ecx: %u\n", tf->ecx);
  printf("edx: %u\n", tf->edx);
  printf("esp: %u\n", tf->esp);
  printf("ebp: %u\n", tf->ebp);
  printf("esi: %u\n", tf->esi);
  printf("edi: %u\n", tf->edi);
  printf("eip: %u\n", tf->eip);
  printf("eflags: %u\n", tf->eflags);
  */

  if (user_handler) {
    _Event ev;
    switch (tf->irq) {
      case IRQ_vectrap: // vectrap
        ev.event = _EVENT_YIELD; break;
      case IRQ_vecsys:
        ev.event = _EVENT_SYSCALL; break;
      case IRQ_TIMER:
        ev.event = _EVENT_IRQ_TIMER; break;
      default: ev.event = _EVENT_ERROR; break;
    }

    next = user_handler(ev, tf);
    if (next == NULL) {
      next = tf;
    }
  }

  _switch(next);
  return next;
}

static GateDesc idt[NR_IRQ];

int _cte_init(_Context*(*handler)(_Event, _Context*)) {
  // initialize IDT
  for (unsigned int i = 0; i < NR_IRQ; i ++) {
    idt[i] = GATE(STS_TG32, KSEL(SEG_KCODE), vecnull, DPL_KERN);
  }

  // -------------------- system call --------------------------
  idt[IRQ_vectrap] = GATE(STS_TG32, KSEL(SEG_KCODE), vectrap, DPL_KERN);
  idt[IRQ_vecsys] = GATE(STS_TG32, KSEL(SEG_KCODE), vecsys, DPL_KERN);
  idt[IRQ_TIMER] = GATE(STS_TG32, KSEL(SEG_KCODE), irq0, DPL_KERN); // irq0

  set_idt(idt, sizeof(idt));

  // register event handler
  user_handler = handler;

  return 0;
}

_Context *_kcontext(_Area stack, void (*entry)(void *), void *arg) {
  _Context *res = ((_Context *)(stack.end - sizeof(_Context)));
  // memset(res, 0, sizeof(_Context));
  res->eip = (uintptr_t) entry;
  res->cs = 8;
  return res;
}

void _yield() {
  asm volatile("int $0x81");
}

int _intr_read() {
  return 0;
}

void _intr_write(int enable) {
}
