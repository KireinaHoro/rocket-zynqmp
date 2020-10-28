#ifndef _RISCV_BITS_H
#define _RISCV_BITS_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "riscv.h"

#define likely(x) __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)

#define ROUNDUP(a, b) ((((a)-1) / (b) + 1) * (b))
#define ROUNDDOWN(a, b) ((a) / (b) * (b))

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define CLAMP(a, lo, hi) MIN(MAX(a, lo), hi)

#define EXTRACT_FIELD(val, which) (((val) & (which)) / ((which) & ~((which)-1)))
#define INSERT_FIELD(val, which, fieldval)                                     \
  (((val) & ~(which)) | ((fieldval) * ((which) & ~((which)-1))))

#define STR(x) XSTR(x)
#define XSTR(x) #x

#ifdef __riscv64
#define SLL32 sllw
#define STORE sd
#define LOAD ld
#define LWU lwu
#define LOG_REGBYTES 3
#else
#define SLL32 sll
#define STORE sw
#define LOAD lw
#define LWU lw
#define LOG_REGBYTES 2
#endif
#define REGBYTES (1 << LOG_REGBYTES)

void _assert(bool x, const char *file, int lineno);
#define assert(x) _assert(x, __FILE__, __LINE__)

#include "mem_map.h"
#include "riscv.h"
#include "spi.h"
#include "uart.h"
#include "gpio.h"
#include "cmd.h"

static inline void delay(int rounds) {
  for (int i = 0; i < rounds; ++i) {
      asm volatile ("nop");
  }
}

static inline void die() {
    while (true) {
        wfi();
    }
}

#define DEBUG_P(...) printf(__VA_ARGS__)
//#define DEBUG_P(...)

void setup_adc(int i, long test_pattern);
void bring_all_adc(long test_pattern);

void setup_synth(int i);
void bring_all_synth();

#endif
