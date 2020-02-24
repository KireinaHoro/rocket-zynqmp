#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "bits.h"
#include "memory.h"
#include "myprintf.h"
#include "uart.h"
#include "riscv.h"
#include "elf.h"
#include "memtest.h"

#define DDR_BASE 0x40000000L
#define DDR_SIZE 0x3ff00000L

volatile uint8_t *mark = (uint8_t *)(DDR_BASE + DDR_SIZE - sizeof(uint8_t));

// SBI ELF is stored in ROM on MMIO bus
#define SBI_BASE 0xf0000000L
#define SBI_SIZE 0x80000

void bootloader(int hartid, void *dtb) {
  if (hartid == 0) {
    uart_init();
    printf(">>> Init on hart 0\r\n", 0);
    printf(">>> OpenSBI ELF at %p", SBI_BASE);
    printf(", size %p bytes\r\n", SBI_SIZE);
    printf(">>> BootROM DTB at %p\r\n", (uint64_t)dtb);
    printf(">>> Setting up machine mode trap...", 0);
    setup_trap();
    printf("done.\r\n", 0);

    printf(">>> Loading OpenSBI ELF...\r\n", 0);
    int ret;
    if ((ret = load_elf((void*)SBI_BASE, SBI_SIZE))) {
      printf("!!! Failed to load ELF: %d\r\n", ret);
      for (;;);
    }
    printf("done.\r\n", 0);
	
    printf(">>> Branching all harts to %p...\r\n", DDR_BASE);
    __sync_synchronize();
    *mark = 1;
  } else {
    while (*mark != 1)
      ;
    __sync_synchronize();
  }

  ((void(*)(int, void*))DDR_BASE)(hartid, dtb);
}
