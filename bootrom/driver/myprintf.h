#ifndef PRINTF_HEADER
#define PRINTF_HEADER

#include <stdint.h>
#include <stddef.h>
#include "uart.h"

// extern void printf(char*, ...);
extern void printf(char *, uint64_t);
void printNum(uint64_t, int);
void putch(uint8_t);

void hexdump(const void* data, size_t size);

#endif
