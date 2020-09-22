#include "riscv.h"
#include "uart.h"

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

int _write(int file, char *ptr, int len) {
  int i;
  file = file;
  for (i = 0; i < len; i++) {
    uart_send(*ptr++);
  }
  return len;
}

void _exit(int n) {
  while (true) {
    wfi();
  }
}

extern intptr_t _heap_start;
extern intptr_t _stack_end;

intptr_t _sbrk(ptrdiff_t heap_incr) {
  static intptr_t heap_end = (intptr_t)&_heap_start;

  intptr_t prev_heap_end;
  intptr_t new_heap_end;

  prev_heap_end = heap_end;
  new_heap_end = prev_heap_end + heap_incr;

  if (new_heap_end >= (intptr_t)&_stack_end) {
    errno = ENOMEM;
    return -1;
  }
  heap_end = new_heap_end;
  return prev_heap_end;
}
