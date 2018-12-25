#include "cpu/exec.h"

make_EHelper(mov) {
  // Warning("mov @%x with %x",id_dest->addr,id_src->val);
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}

make_EHelper(push) {
  if(decoding.is_operand_size_16) {
    TODO();
  }
  else {
    if(id_dest->type == OP_TYPE_IMM && id_dest->width < 4)
      rtl_sext(&id_dest->val,&id_dest->val,id_dest->width);
    rtl_push(&id_dest->val);
  }

  print_asm_template1(push);
}

make_EHelper(pop) {
  if(decoding.is_operand_size_16) {
    TODO();
  }
  else {
    rtl_pop(&t0);
    operand_write(id_dest,&t0);
  }

  print_asm_template1(pop);
}

make_EHelper(pusha) {
  if(decoding.is_operand_size_16) {
    TODO();
  }
  else {
    t0 = cpu.esp;
    rtl_push(&cpu.eax);
    rtl_push(&cpu.ecx);
    rtl_push(&cpu.edx);
    rtl_push(&cpu.ebx);
    rtl_push(&t0);  // esp
    rtl_push(&cpu.ebp);
    rtl_push(&cpu.esi);
    rtl_push(&cpu.edi);

    /*
    // Check _Context
    printf("ori eax: %u\n", cpu.eax);
    printf("ori ebx: %u\n", cpu.ebx);
    printf("ori ecx: %u\n", cpu.ecx);
    printf("ori edx: %u\n", cpu.edx);
    printf("ori esp: %u\n", t0);
    printf("ori ebp: %u\n", cpu.ebp);
    printf("ori esi: %u\n", cpu.esi);
    printf("ori edi: %u\n", cpu.edi);
    printf("ori eip: %u\n", cpu.eip);
    printf("ori eflags: %u\n", cpu.eflags);
    */
  }

  print_asm("pusha");
}

make_EHelper(popa) {
  if(decoding.is_operand_size_16) {
    TODO();
  }
  else {
    rtl_pop(&cpu.edi);
    rtl_pop(&cpu.esi);
    rtl_pop(&cpu.ebp);
    rtl_pop(&t0);
    rtl_pop(&cpu.ebx);
    rtl_pop(&cpu.edx);
    rtl_pop(&cpu.ecx);
    rtl_pop(&cpu.eax);
  }

  print_asm("popa");
}

make_EHelper(leave) {
  cpu.esp = cpu.ebp;
  if(decoding.is_operand_size_16){
    TODO();
  }
  else{
    rtl_pop(&cpu.ebp);
  }

  print_asm("leave");
}

make_EHelper(cltd) {
  if (decoding.is_operand_size_16) {
    rtl_msb(&t0, &reg_l(R_AX), 2);
    if(t0 == 1)
      reg_w(R_DX) = 0xffff;
    else reg_w(R_DX) = 0;
  }
  else {
    rtl_msb(&t0, &reg_l(R_EAX), 4);
    if(t0 == 1){
      reg_l(R_EDX) = 0xffffffff;
    }
    else reg_l(R_EDX) = 0;
  }

  print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {
  if (decoding.is_operand_size_16) {
    rtl_sext(&reg_l(R_AX), &reg_l(R_AL), 1);
  }
  else {
    rtl_sext(&reg_l(R_EAX), &reg_l(R_AX), 2);
  }

  print_asm(decoding.is_operand_size_16 ? "cbtw" : "cwtl");
}

make_EHelper(movsx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_sext(&t0, &id_src->val, id_src->width);
  operand_write(id_dest, &t0);
  print_asm_template2(movsx);
}

make_EHelper(movzx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  operand_write(id_dest, &id_src->val);
  print_asm_template2(movzx);
}

make_EHelper(lea) {
  if(decoding.is_operand_size_16){
    TODO();
  }
  else{
    operand_write(id_dest, &id_src->addr);
  }
  print_asm_template2(lea);
}
