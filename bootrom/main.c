#include "bits.h"

void main(int hartid, void *dtb) {
  if (hartid == 0) {
    uart_init();
    // trap requires UART
    setup_trap();
    printf("\n>>> Init on hart 0\n");
    printf(">>> BootROM DTB at %p\n", dtb);
    printf(">>> SPI controller setup\n");
    spi_init();
    printf(">>> Device startup\n");


    printf(">>> Jobs done, spinning\n");
    goto hang;
  } else {
    goto hang;
  }
hang:
  while (true) {
    wfi();
  }
}
