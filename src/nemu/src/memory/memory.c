#include "nemu.h"
#include "device/mmio.h"
#include "memory/mmu.h"

#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */

uint32_t paddr_read(paddr_t addr, int len) {
  int mmio_id = is_mmio(addr);
  if(mmio_id == -1){
    return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
  }
  else{
    return mmio_read(addr, len, mmio_id);
  }
}

void paddr_write(paddr_t addr, uint32_t data, int len) {
  int mmio_id = is_mmio(addr);
  if (mmio_id == -1){
    memcpy(guest_to_host(addr), &data, len);
  }
  else{
    mmio_write(addr, len, data, mmio_id);
  }
}

typedef union {
  struct{
    uint32_t offset : 12;
    uint32_t page : 10;
    uint32_t dir : 10;
  };
  uint32_t raw;
} LinearAddress;

paddr_t page_translate(LinearAddress addr){
  
  if (addr.dir >= NR_PDE){
    panic("Too large dir index");
  }
  
  PDE d_entry; 
  d_entry.val = paddr_read((cpu.cr3.page_directory_base << 12) + addr.dir * sizeof(PDE), 4);

  if (d_entry.present == 0){
    Log("addr: %x pdb %x %x %x", addr.raw, cpu.cr3.page_directory_base << 12, addr.dir, d_entry.val);
    panic("Present bit Zero for dir_entry");
  }

  if (addr.page >= NR_PTE){
    panic("Too large page index");
  }
  PTE t_entry;
  t_entry.val = paddr_read((d_entry.page_frame << 12) + addr.page * sizeof(PTE), 4);

  if (t_entry.present == 0){
    panic("Present bit Zero for table_entry");
  }

  return (t_entry.page_frame << 12) | addr.offset;
}

uint32_t vaddr_read(vaddr_t addr, int len) {
  // Log("PG: %d", cpu.cr0.pg);
  if (cpu.cr0.protect_enable == 1 && cpu.cr0.paging == 1){ // Enable page trans
    LinearAddress l, r;
    l.raw = addr;
    r.raw = addr + len - 1;
    if (l.dir != r.dir || l.page != r.page) {
      // Warning("data cross the page boundary %x l(%u,%u) r(%u,%u)", addr, l.dir, l.page, r.dir, r.page);

      vaddr_t addr_mid = r.raw & (~0xfff);
      int len_l = addr_mid - addr, len_r = len - len_l;
      LinearAddress mid;
      mid.raw = addr_mid;
      uint32_t data_l = paddr_read(page_translate(l), len_l);
      uint32_t data_r = paddr_read(page_translate(mid), len_r);
      return (data_l << (len_r << 3)) | data_r;
    }
    else {
      paddr_t paddr = page_translate(l);
      return paddr_read(paddr, len);
      // Warning("Translated: %x", paddr);
    }
  }
  else {
    return paddr_read(addr, len);
  }
}

void vaddr_write(vaddr_t addr, uint32_t data, int len) {
  if (cpu.cr0.protect_enable == 1 && cpu.cr0.paging == 1){ // Enable page trans
    LinearAddress l, r;
    l.raw = addr;
    r.raw = addr + len - 1;
    if (l.dir != r.dir || l.page != r.page) {
      // Warning("data cross the page boundary %x l(%u,%u) r(%u,%u)", addr, l.dir, l.page, r.dir, r.page);

      vaddr_t addr_mid = r.raw & (~0xfff);
      int len_l = addr_mid - addr, len_r = len - len_l;
      LinearAddress mid;
      mid.raw = addr_mid;
      paddr_write(page_translate(l), data >> (len_r << 3), len_l);
      paddr_write(page_translate(mid), data & (~0u >> ((4 - len_r) << 3)), len_r);
    }
    else {
      paddr_t paddr = page_translate(l);
      paddr_write(paddr, data, len);
      // Warning("Translated: %x", paddr);
    }
  }
  else {
    paddr_write(addr, data, len);
  }
}
