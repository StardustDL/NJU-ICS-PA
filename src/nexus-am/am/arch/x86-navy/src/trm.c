#include <am.h>

#include <stdio.h>
#include <stdlib.h>

// extern char _heap_start;
// extern char _heap_end;
extern int main();

void _trm_init() {
  int ret = main();
  _halt(ret);
}

void _putc(char ch) {
    putchar(ch);
}

void _halt(int code) {
    exit(code);

    // should not reach here
    while (1);
}

_Area _heap;
