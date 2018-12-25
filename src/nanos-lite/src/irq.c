#include "common.h"

_Context* do_syscall(_Context *c);
_Context* schedule(_Context *prev);

static _Context* do_event(_Event e, _Context* c) {
  switch (e.event) {
    case _EVENT_YIELD:
      // Log("IRQ Event: %s", "Recieved YIELD event.");
      return schedule(c);
      break;
    case _EVENT_SYSCALL:
      // printf("IRQ Event: %s\n", "Recieved SYSCALL event.");
      return do_syscall(c);
      break;
    case _EVENT_IRQ_TIMER:
      // Log("IRQ Event: %s", "Recieved TIMER event.");
      _yield();
      break;
    default: panic("Unhandled event ID = %d", e.event);
  }

  return NULL;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  _cte_init(do_event);
}
