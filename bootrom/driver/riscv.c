// code borrowed from xv6-riscv with modifications for machine mode

#include "riscv.h"
#include "bits.h"

// Machine Trap Cause
static inline uint64 r_mcause() {
  uint64 x;
  asm volatile("csrr %0, mcause" : "=r"(x));
  return x;
}

// Machine Trap Value
static inline uint64 r_mtval() {
  uint64 x;
  asm volatile("csrr %0, mtval" : "=r"(x));
  return x;
}

static inline uint64 r_mepc() {
  uint64 x;
  asm volatile("csrr %0, mepc" : "=r"(x));
  return x;
}

static const char *mcause_desc(uint64 mtval) {
  static const char *intr_desc[16] = {
      [0] "user software interrupt",
      [1] "supervisor software interrupt",
      [2] "<reserved for future standard use>",
      [3] "machine software interrupt",
      [4] "user timer interrupt",
      [5] "supervisor timer interrupt",
      [6] "<reserved for future standard use>",
      [7] "machine timer interrupt",
      [8] "user external interrupt",
      [9] "supervisor external interrupt",
      [10] "<reserved for future standard use>",
      [11] "machine external interrupt",
      [12] "<reserved for future standard use>",
      [13] "<reserved for future standard use>",
      [14] "<reserved for future standard use>",
      [15] "<reserved for future standard use>",
  };
  static const char *nointr_desc[16] = {
      [0] "instruction address misaligned",
      [1] "instruction access fault",
      [2] "illegal instruction",
      [3] "breakpoint",
      [4] "load address misaligned",
      [5] "load access fault",
      [6] "store/AMO address misaligned",
      [7] "store/AMO access fault",
      [8] "environment call from U-mode",
      [9] "environment call from S-mode",
      [10] "<reserved for future standard use>",
      [11] "environment call from M-mode",
      [12] "instruction page fault",
      [13] "load page fault",
      [14] "<reserved for future standard use>",
      [15] "store/AMO page fault",
  };
  uint64 interrupt = mtval & 0x8000000000000000L;
  uint64 code = mtval & ~0x8000000000000000L;
  if (interrupt) {
    if (code < NELEM(intr_desc)) {
      return intr_desc[code];
    } else {
      return "<reserved for platform use>";
    }
  } else {
    if (code < NELEM(nointr_desc)) {
      return nointr_desc[code];
    } else if (code <= 23) {
      return "<reserved for future standard use>";
    } else if (code <= 31) {
      return "<reserved for custom use>";
    } else if (code <= 47) {
      return "<reserved for future standard use>";
    } else if (code <= 63) {
      return "<reserved for custom use>";
    } else {
      return "<reserved for future standard use>";
    }
  }
}

// print trap information, halt
// assumes uart is already initialized
void mtrapvec(void) {
  uint64 mcause = r_mcause();
  uint64 mepc = r_mepc();
  printf("!!! Machine mode trap: mcause %#lx (%s)\n    mepc=%#lx mtval=%#lx\n",
         mcause, mcause_desc(mcause), mepc, r_mtval());

  while (true)
    wfi();
}

void setup_trap(void) { w_mtvec((uint64)mtrapvec); }
