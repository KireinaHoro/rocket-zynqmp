// reserve place for bootloader sync mark and stay aligned
#define DRAM_TOP 0x7ff00000

.section .text.start, "ax", @progbits
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
  li sp, DRAM_TOP - 0x10    // setup stack
  mv t0, a0
  slli t0, t0, 12    // 4K stack for each hart
  sub sp, sp, t0
  la a1, _dtb        // dtb address for next level bootloader
  li t0, DRAM_TOP - 1  // address for MARK in bootloader.c
  sb zero, 0(t0)
  jal bootloader

.section .rodata.dtb, "a", @progbits
.globl _dtb
.balign 4
_dtb:
//.string "Hello, world!"
