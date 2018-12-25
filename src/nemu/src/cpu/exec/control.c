#include "cpu/exec.h"
#include "cpu/cc.h"

make_EHelper(jmp) {
  // Warning("%x",id_dest->width);
  // the target address is calculated at the decode stage
  rtl_j(decoding.jmp_eip);

  if(decoding.is_operand_size_16)
    TODO();

  print_asm("jmp %x", decoding.jmp_eip);// the target address is calculated at the decode stage
}

make_EHelper(jcc) {
  // the target address is calculated at the decode stage
  uint32_t cc = decoding.opcode & 0xf;
  rtl_setcc(&t0, cc);
  rtl_li(&t1, 0);
  rtl_jrelop(RELOP_NE, &t0, &t1, decoding.jmp_eip);
  
  print_asm("j%s %x", get_cc_name(cc), decoding.jmp_eip);
}

make_EHelper(jmp_rm) {
  rtl_jr(&id_dest->val);

  if(decoding.is_operand_size_16)
    TODO();

  print_asm("jmp *%s", id_dest->str);
}

make_EHelper(call) {

#ifdef DEBUG
  // Info("call %x %x",cpu.eip,decoding.jmp_eip);
#endif

  // Info("call %x %x",cpu.eip,decoding.jmp_eip);

  // the target address is calculated at the decode stage
  if(decoding.is_operand_size_16) {
    TODO();
  }
  else {
    // rtl_push(&cpu.eip);
    rtl_push(&decoding.seq_eip); // Push the next instr position
    rtl_j(decoding.jmp_eip);
    Assert(cpu.eip == decoding.jmp_eip,"fail jump");
  }
  
  print_asm("call %x", decoding.jmp_eip);
}

make_EHelper(ret) {
  
  // int t=cpu.eip==0x102288;

  // C3
  if(decoding.is_operand_size_16) {
    TODO();
  }
  else{
    rtl_pop(&t0);
    rtl_j(t0);
    // If has immediate TODO()
  }

  // Info("ret %x",cpu.eip);

  print_asm("ret");
}

make_EHelper(call_rm) {
  #ifdef DEBUG
    // Info("call %x *%s = %x",cpu.eip,id_dest->str,id_dest->val);
  #endif

  // Info("call %x *%s = %x",cpu.eip,id_dest->str,id_dest->val);

  if(decoding.is_operand_size_16) {
    TODO();
  }
  else {
    // rtl_push(&cpu.eip);
    rtl_push(&decoding.seq_eip); // Push the next instr position
    rtl_jr(&id_dest->val);
  }

  print_asm("call *%s", id_dest->str);
}
