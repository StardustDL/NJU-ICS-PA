#include "proc.h"

#define MAX_NR_PROC 4

void naive_uload(PCB *pcb, const char *filename);
void context_kload(PCB *pcb, void *entry);
void context_uload(PCB *pcb, const char *filename);

static PCB pcb[MAX_NR_PROC] __attribute__((used));
static PCB pcb_boot;
PCB *current;
static int current_id;
static int fg_pcb_id;
static uint32_t current_cnt;

int f1_pcb, f2_pcb, f3_pcb;

static int empty_proc(){
  for (int i = 0; i < MAX_NR_PROC; i++){
    if (pcb[i].priority == 0)
      return i;
  }
  panic("No empty proc in pool");
}

static int next_proc(){
  for (int i = 1; i <= MAX_NR_PROC; i++){
    int tid = (current_id + i) % MAX_NR_PROC;
    if (pcb[tid].priority)
      return tid;
  }
  panic("No running proc in pool");
}

void set_foreground(int id) {
  fg_pcb_id = id;
}

int new_proc(const char *filename, uint32_t priority) {
  int id = empty_proc();
  context_uload(&pcb[id], filename);
  pcb[id].priority = priority;
  return id;
}

void exit_proc(){
  pcb[current_id].priority = 0;
  _yield();
}

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite for the %dth time!", j);
    j ++;
    _yield();
  }
}

void init_proc() {
  // naive_uload(NULL, NULL);
  // naive_uload(NULL, "/bin/text");
  // naive_uload(NULL, "/bin/bmptest");
  // naive_uload(NULL, "/bin/events");
  // naive_uload(NULL, "/bin/pal");
  // naive_uload(NULL, "/bin/init");
  // context_kload(&pcb[0], (void *)hello_fun);
  // context_uload(&pcb[0], "/bin/dummy");
  // current_id = new_proc("/bin/events", 1);
  current_id = new_proc("/bin/hello", 1);
  f1_pcb = new_proc("/bin/pal",100);
  f2_pcb = new_proc("/bin/slider-am",100);
  f3_pcb = new_proc("/bin/typing-am",100);
  set_foreground(f1_pcb);
  // current_id = new_proc("/bin/microbench-am",10); // Failed for lzip assert
  current_cnt = 0;
  // new_proc("/bin/init", 10);
  
  if (false) {
    next_proc(); // Because the show, don't use this
  }
  
  // context_uload(&pcb[1], "/bin/pal");
  switch_boot_pcb();
}

_Context* schedule(_Context *prev) {
  current->cp = prev;

  // current = &pcb[0];
  if (current_cnt < current->priority) {
    current_cnt++;
  }
  else {
    // current_id = next_proc();
    if (current_id == fg_pcb_id) {
      current_id = 0;
    }
    else {
      current_id = fg_pcb_id;
    }
    current = &pcb[current_id];
    current_cnt = 0;
  }
  // current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
  return current->cp;
}
