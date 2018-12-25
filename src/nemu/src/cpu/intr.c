#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */

  rtl_push(&cpu.eflags);

  cpu.IF = 0; // Set for disable intr

  rtl_push(&cpu.cs);
  
  rtl_push(&ret_addr); // rtl_push(&cpu.eip);

  vaddr_t offset = NO * sizeof(GateDesc);

  Assert(offset < cpu.idtr.limit, "Bigger than IDTR limit.");

  GateDesc gate;
  gate.val0 = vaddr_read(cpu.idtr.base + offset, 4);
  gate.val1 = vaddr_read(cpu.idtr.base + offset + 4, 4);

  // Warning("Gat offset %u\n",gate.offset_31_16 << 16 | gate.offset_15_0);
  rtl_j(gate.offset_31_16 << 16 | gate.offset_15_0);

  // Warning("Not use ret_addr argument.");
}

void dev_raise_intr() {
  cpu.intr = true;
}
