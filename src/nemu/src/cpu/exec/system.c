#include "cpu/exec.h"
#include "device/port-io.h"

void difftest_skip_ref();
void difftest_skip_dut();
void raise_intr(uint8_t NO, vaddr_t ret_addr);

make_EHelper(lidt) {
  if(decoding.is_operand_size_16) {
    TODO();
  }
  else {
    t0 = id_dest->addr;
    rtl_lm(&t1, &t0, 2);
    cpu.idtr.limit = t1;

    t0 += 2;
    rtl_lm(&t1, &t0, 4);
    cpu.idtr.base = t1;

    // Warning("limit: %u base: %u",cpu.idtr.limit, cpu.idtr.base);
  }

  print_asm_template1(lidt);
}

make_EHelper(mov_r2cr) {
  // Info("cr%d val: %x", id_dest->reg, id_src->val);

  cpu.cr[id_dest->reg] = id_src->val;

  print_asm("movl %%%s,%%cr%d", reg_name(id_src->reg, 4), id_dest->reg);
}

make_EHelper(mov_cr2r) {
  // Info("cr%d val: %x", id_src->reg, cpu.cr[id_src->reg]);

  operand_write(id_dest, &(cpu.cr[id_src->reg]));

  print_asm("movl %%cr%d,%%%s", id_src->reg, reg_name(id_dest->reg, 4));

#if defined(DIFF_TEST)
  difftest_skip_ref();
#endif
}

make_EHelper(int) {
  // Warning("int: %u", id_dest->val);

  raise_intr(id_dest->val, decoding.seq_eip);

  print_asm("int %s", id_dest->str);

#if defined(DIFF_TEST) && defined(DIFF_TEST_QEMU)
  difftest_skip_dut();
#endif
}

make_EHelper(iret) {
  if(decoding.is_operand_size_16) {
    TODO();
  }
  else {
    rtl_pop(&t0);
  }
  rtl_pop(&cpu.cs);
  if(decoding.is_operand_size_16) {
    TODO();
  }
  else {
    rtl_pop(&cpu.eflags);
  }

  rtl_j(t0);

  print_asm("iret");
}

make_EHelper(in) {

  /*if(id_src->type == OP_TYPE_IMM){
    TODO();
  }
  else{*/
  switch(id_dest->width){
    case 1:
      t2 = pio_read_b((ioaddr_t)(id_src->val));
      break;
    case 2:
      t2 = pio_read_w((ioaddr_t)(id_src->val));
      break;
    case 4:
      t2 = pio_read_l((ioaddr_t)(id_src->val));
      break;
    default:
      Assert(0,"in: Unexpected src width");
      break;
  }

  // Warning("in %x:%x -> @%s",id_src->val,t2,id_dest->str);

  operand_write(id_dest, &t2);
  //}

  print_asm_template2(in);

#if defined(DIFF_TEST)
  difftest_skip_ref();
#endif
}

make_EHelper(out) {

  // Warning("out %x -> @%x",id_src->val,id_dest->val);

  switch(id_dest->width){
    case 1:
      pio_write_b((ioaddr_t)(id_dest->val), id_src->val);
      break;
    case 2:
      pio_write_w((ioaddr_t)(id_dest->val), id_src->val);
      break;
    case 4:
      pio_write_l((ioaddr_t)(id_dest->val), id_src->val);
      break;
    default:
      Assert(0,"out: Unexpected src width");
      break;
  }

  print_asm_template2(out);

#if defined(DIFF_TEST)
  difftest_skip_ref();
#endif
}
