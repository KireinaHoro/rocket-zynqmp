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

#define BRAM_BASE 0x40000L
#define BRAM_SIZE 0x20000L

#define DDR_BASE 0x40000000L
#define DDR_SIZE 0x40000000L

volatile uint8_t *mark = (uint8_t *)(BRAM_BASE + BRAM_SIZE - sizeof(uint8_t));

extern char _sbi;
extern int _sbi_size;

void bootloader(int hartid, void *dtb) {
  if (hartid == 0) {
    uart_init();
    printf(">>> Init on hart 0\r\n", 0);
    printf(">>> OpenSBI ELF at %p", (uint64_t)&_sbi);
    printf(", size %p bytes\r\n", _sbi_size);
    printf(">>> BootROM DTB at %p\r\n", (uint64_t)dtb);
    printf(">>> Setting up machine mode trap...", 0);
    setup_trap();
    printf("done.\r\n", 0);

    printf(">>> Loading OpenSBI ELF...\r\n", 0);
    int ret;
    if ((ret = load_elf((void*)&_sbi, _sbi_size))) {
      printf("!!! Failed to load ELF: %d\r\n", ret);
      for (;;);
    }
    printf("done.\r\n", 0);
	
	printf(">>> Performing memtest at base %p, ", DDR_BASE);
	printf("size %p...\r\n", DDR_SIZE);

	memtest((void*)DDR_BASE, DDR_SIZE);

	printf(">>> Memtest done.\r\n", 0);

    printf(">>> Branching all harts to %p...\r\n", BRAM_BASE);
    __sync_synchronize();
    *mark = 1;
  } else {
    while (*mark != 1)
      ;
    __sync_synchronize();
  }

  ((void(*)(int, void*))BRAM_BASE)(hartid, dtb);
}
