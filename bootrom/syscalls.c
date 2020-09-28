#include "riscv.h"
#include "uart.h"

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

int _write(int file, char *ptr, int len) {
  int i;
  file = file;
  for (i = 0; i < len; i++) {
    char c = *ptr++;
    if (c == '\n') {
      uart_send('\r');
    }
    uart_send(c);
  }
  return len;
}

void _exit(int n) {
  while (true) {
    wfi();
  }
}

extern intptr_t _heap_start;
intptr_t _sbrk(ptrdiff_t heap_incr) {
  static intptr_t heap_end = (intptr_t)&_heap_start;

  intptr_t prev_heap_end;
  intptr_t new_heap_end;

  prev_heap_end = heap_end;
  new_heap_end = prev_heap_end + heap_incr;

  if (new_heap_end >= (intptr_t)STACK_END) {
    errno = ENOMEM;
    return -1;
  }
  heap_end = new_heap_end;
  return prev_heap_end;
}
int _read(int fd, char *buff, int size) { return 0; }

int _open(const char *name, int flags, int mode) { return 0; }

void _close(int fd) {}
int _isatty(int fd) { return 1; }
int _fstat(int fd, struct stat *st) {
  st->st_mode = S_IFCHR;
  return 0;
}
off_t _lseek(int fd, off_t offset, int whence) { return 0; }
