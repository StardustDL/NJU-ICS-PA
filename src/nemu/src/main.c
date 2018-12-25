#include <stdio.h>
#include <stdint.h>
#include "../include/monitor/expr.h"
#include "nemu.h"

void init_regex();

int init_monitor(int, char *[]);
void ui_mainloop(int);

int main(int argc, char *argv[]) {
//#define TEST_EXPR
#ifdef TEST_EXPR
Log("Begin test expr");
init_regex();
//FILE* finp=fopen("../tools/gen-expr/input","r");
//Log("Open file: %d",finp!=NULL);
static char s[70000];
uint32_t exp=0;
bool success;
while(scanf("%u %s\n",&exp,s)==2){
	Log("Readed %u %s",exp,s);
	uint32_t rel=expr(s,&success);
	if(success == false || rel!=exp){
		Log("Failed: Expected %u but real %u",exp,rel);
	}
//	else Log("Success: %d == %d",exp,rel);
}
//fclose(finp);
return 0;
#endif

 
 /* Initialize the monitor. */
  int is_batch_mode = init_monitor(argc, argv);

  /* Receive commands from user. */
  ui_mainloop(is_batch_mode);

  return 0;
}
