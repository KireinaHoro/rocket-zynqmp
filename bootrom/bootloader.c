#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "myprintf.h"
#include "bits.h"
#include "diskio.h"
#include "ff.h"
#include "uart.h"
#include "elf.h"
#include "memory.h"
#include "spi.h"

FATFS fat_fs;

#define DDR_BASE 0x800000000
#define DDR_SIZE 0x80000000
#define SD_READ_SIZE 4096

volatile uint8_t *mark = (uint8_t *)(DDR_BASE + DDR_SIZE - sizeof(uint8_t));

void bootloader(int hartid, void *dtb) {
  uint8_t *load_location = (uint8_t *)(DDR_BASE);
  if (hartid == 0) {
    FIL fil;
    uart_init();
    printf("Init on hart 0\r\n", 0);
    printf("BootROM DTB at %p\r\n", (uint64_t)dtb);
    printf("Mounting FAT on SPI SD...\r\n", 0);
    if (f_mount(&fat_fs, "", 1)) {
      printf("!!! Failed to mount FAT filesystem\r\n", 0);
      while (true)
        ;
    }
    printf("Loading BOOT.BIN to %p...", (uint64_t)load_location);
    if (f_open(&fil, "BOOT.BIN", FA_READ)) {
      printf("!!! Failed to open BOOT.BIN\r\n", 0);
      while (true)
        ;
    }
    uint32_t fsize = 0;
    uint32_t br;
    FRESULT fr;
    uint8_t *buf = load_location;
    do {
      fr = f_read(&fil, buf, SD_READ_SIZE, &br);
      buf += br;
      fsize += br;
    } while (!(fr || br == 0));
    printf("loaded %d bytes.\r\n", fsize);
    if (f_close(&fil)) {
      printf("!!! Failed to close BOOT.BIN\r\n", 0);
      while (true)
        ;
    }
    if (f_mount(NULL, "", 1)) {
      printf("!!! Failed to umount FAT filesystem\r\n", 0);
      while (true)
        ;
    }
    printf("Branching all harts to %p...", (uint64_t)load_location);
    __sync_synchronize();
    *mark = 1;
  } else {
    while (*mark != 1)
      ;
    __sync_synchronize();
  }

  ((void (*)(int, void *))load_location)(hartid, dtb);
}
