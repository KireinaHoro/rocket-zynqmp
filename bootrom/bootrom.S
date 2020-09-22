#include "driver/mem_map.h"

.section .start, "ax", @progbits
.globl _start
_start:
  li a1, 0x2000000 // base address of clint
  csrr a0, mhartid   // hart 0 continues to setup interrupt
  bnez a0, boot_core // everyone else wait

  addi a2, a1, 4   // 0x20000004 hart 0 msip
  li a3, 1
interrupt_loop:
  sw a3, 0(a2)     // enable interrupt
  addi a2, a2, 4
  lw a3, -4(a2)
  bnez a3, interrupt_loop  // was that the last core?
  j boot_core_hart0

boot_core:
  lw t0, 0(a1) // load hart 0 msip
  bnez t0, boot_core // block until hart 0 clears own msip
  sll a0, a0, 2 // offset for hart msip
  add a1, a0, a1
boot_core_hart0:
  sw zero, 0(a1)     // clear the interrupt
  csrr a0, mhartid   // hartid for next level bootloader
  li sp, DDR_BASE + DDR_SIZE
  mv t0, a0
  slli t0, t0, 12    // 4K stack for each hart
  sub sp, sp, t0

  la t0, _real_data
  la t1, _data_start
  la t2, _data_end
  sub t2, t2, t1
relocate_data:
  ld t3, 0(t0)
  sd t3, 0(t1)
  addi t0, t0, 8
  addi t1, t1, 8
  addi t2, t2, -8
  bnez t2, relocate_data

  la t0, _bss_start
  la t1, _sbss_end
zero_bss:
  sd zero, 0(t0)
  addi t0, t0, 8
  bne t0, t1, zero_bss

  la a1, _dtb        // dtb address for next level bootloader
  sb zero, 0(t0)
  call main
  tail exit

.section .rodata.dtb, "a", @progbits
.globl _dtb
.balign 4
_dtb:
//.string "Hello, world!"
