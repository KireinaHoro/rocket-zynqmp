#include "bits.h"

void main(int hartid, void *dtb) {
  int bitslip = 0;
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
      // GPIO LED counter
      printf("Current Bitslip: %d\n", bitslip);
      printf("Enter new bitslip: ");
      fflush(stdout);
      int temp;
      int ret = scanf("%d", &temp);
      if (ret <= 0) {
          printf("stdin got clogged up, flushing...\n");
          fflush(stdin);
          continue;
      }
      if (temp < 0 || temp >= 8) {
          printf("Invalid bitslip %d: must be in [0,8)\n", temp);
      } else {
          bitslip = temp;
          write_gpio_reg(bitslip << 5);
      }
  }
}
