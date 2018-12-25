#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536];
static int bufPos;
static char ops[]={'+','-','*','/'};
static int choose(int mx){
	return rand()%mx;
}
static inline void gen(char c){
	buf[bufPos++]=c;
}
static inline void gen_num(){
	int len=choose(6);
	gen(choose(9)+'1');
	while(len--){
		gen(choose(10)+'0');
	}
}
static inline void gen_op(){
	gen(ops[choose(sizeof(ops)/sizeof(char))]);
}
const int MAXN=10;
static inline void gen_rand_expr() {
	  switch(choose(3)){
			case 0: gen_num(); break;
			case 1: gen('('); gen_rand_expr(); gen(')'); break;
		  default: gen_rand_expr(); gen_op(); gen_rand_expr(); break;
		}
}

static char code_buf[65536];
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
		bufPos = 0;
    gen_rand_expr();
		buf[bufPos]='\0';
    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen(".code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc .code.c -o .expr");
    if (ret != 0) continue;

    fp = popen("./.expr", "r");
    assert(fp != NULL);

    int result;
    fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
