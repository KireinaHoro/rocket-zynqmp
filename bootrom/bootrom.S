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
  li sp, STACK_TOP
  mv t0, a0
  slli t0, t0, 12    // 4K stack for each hart
  sub sp, sp, t0

  la a0, _data_start
  la a2, _data_end
  sub a2, a2, a0
  la a1, _real_data
  call memcpy

  la a0, _bss_start
  la a2, _bss_end
  sub a2, a2, a0
  li a1, 0
  call memset

  call __libc_init_array

  la a1, _dtb        // dtb address for next level bootloader
  csrr a0, mhartid   // hartid for next level bootloader
  call main
  tail exit

.section .rodata.dtb, "a", @progbits
.globl _dtb
.balign 4
_dtb:
//.string "Hello, world!"
