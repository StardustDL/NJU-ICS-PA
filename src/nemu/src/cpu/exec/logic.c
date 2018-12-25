#include "cpu/exec.h"
#include "cpu/cc.h"

extern rtlreg_t tzero;

make_EHelper(test) {
  // 85 /r
  rtl_and(&t2, &id_dest->val, &id_src->val);
  // operand_write(id_dest, &t2);

  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);
  rtl_update_ZFSF(&t2, id_dest->width);
  print_asm_template2(test);
}

make_EHelper(and) {
  
  // 83 /4 ib AND r/m32 imm8
  if(id_src->width == 1 && id_dest->width > 1){
    rtl_sext(&id_src->val, &id_src->val, id_src->width);
  }

  rtl_andi(&t2, &id_dest->val, id_src->val);
  operand_write(id_dest, &t2);

  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);
  rtl_update_ZFSF(&t2, id_dest->width);

  print_asm_template2(and);
}

make_EHelper(xor) {
  // Info("xor %d %d %d",id_src->val,id_dest->val,id_src->val^id_dest->val);

  // 33 /r 31 /r
  if(id_src->width == 1 && id_dest->width > 1){
    rtl_sext(&id_src->val, &id_src->val, id_src->width);
  }

  rtl_xor(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);

  // Warning("xor %x",t2);

  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);
  rtl_update_ZFSF(&t2,id_dest->width);
  // Info("xor %d %d",id_dest->val,cpu.eax);

  print_asm_template2(xor);
}

make_EHelper(or) {
  // 09 /r
  if(id_src->width == 1 && id_dest->width > 1){
    rtl_sext(&id_src->val, &id_src->val, id_src->width);
  }

  rtl_or(&t2,&id_dest->val,&id_src->val);
  operand_write(id_dest, &t2);

  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);
  rtl_update_ZFSF(&t2,id_dest->width);

  print_asm_template2(or);
}

make_EHelper(sar) {
  // c1 /7 ib

  // unnecessary to update CF and OF in NEMU

  rtl_sar(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);

  rtl_update_ZFSF(&t2, id_dest->width);

  print_asm_template2(sar);
}

make_EHelper(shl) {
  // d3 /4

  // unnecessary to update CF and OF in NEMU
  /*
  Assert(id_src->val > 0, "Not a valid shift len.");

  t0 = id_src->val - 1;
  rtl_shl(&t2, &id_dest->val, &t0);
  rtl_msb(&t1, &t2, id_dest->width);
  rtl_set_CF(&t1);
  rtl_shli(&t2, &t2, 1);
  if(id_src->val == 1){
    Assert(id_src->type == OP_TYPE_IMM, "1 bit shift must be imm");
    rtl_msb(&t3, &t2, id_dest->width);
    rtl_xor(&t0, &t1, &t3);
    rtl_set_OF(&t0);
  }*/

  rtl_shl(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);

  rtl_update_ZFSF(&t2, id_dest->width);

  print_asm_template2(shl);
}

make_EHelper(shr) {
  rtl_shr(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);

  // unnecessary to update CF and OF in NEMU

  rtl_update_ZFSF(&t2, id_dest->width);

  print_asm_template2(shr);
}

make_EHelper(setcc) {
  uint32_t cc = decoding.opcode & 0xf;

  rtl_setcc(&t2, cc);
  operand_write(id_dest, &t2);

  print_asm("set%s %s", get_cc_name(cc), id_dest->str);
}

make_EHelper(not) {
  // F7 /2

  rtl_not(&t2, &id_dest->val);

  operand_write(id_dest, &t2);

  // Warning("not instr NOT change eflags");

  print_asm_template1(not);
}

make_EHelper(rol) {

  t0 = id_src->val;
  t2 = id_dest->val;
  while(t0){
    rtl_msb(&t1, &t2, id_dest->width);
    rtl_set_CF(&t1);
    rtl_shli(&t2, &t2, 1);
    rtl_or(&t2, &t2, &t1);
    t0--;
  }
  if(id_src->val == 1){
    rtl_msb(&t3, &t2, id_dest->width);
    rtl_xor(&t0, &t1, &t3);
    rtl_set_OF(&t0);
  }

  operand_write(id_dest, &t2);
  rtl_update_ZFSF(&t2, id_dest->width);

  print_asm_template2(rol);
}

make_EHelper(ror) {

  t0 = id_src->val;
  t2 = id_dest->val;
  while(t0){
    rtl_andi(&t1, &t2, 1);
    rtl_set_CF(&t1);
    rtl_shri(&t2, &t2, 1);
    rtl_shli(&t1, &t1, id_dest->width - 1);
    rtl_or(&t2, &t2, &t1);
    t0--;
  }
  if(id_src->val == 1){
    rtl_msb(&t3, &t2, id_dest->width);
    rtl_shli(&t1, &t2, 1);
    rtl_msb(&t1, &t1, id_dest->width);
    rtl_xor(&t0, &t1, &t3);
    rtl_set_OF(&t0);
  }

  operand_write(id_dest, &t2);
  rtl_update_ZFSF(&t2, id_dest->width);

  print_asm_template2(rol);
}