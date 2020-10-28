#include "bits.h"

#define PATTERN 0x2a0c

void main(int hartid, void *dtb) {
  int bitslip = 0;
  init_cmd();
  int cmd = recv_cmd();
  if (hartid == 0) {
    uart_init();
    // trap requires UART
    setup_trap();
    printf("\n>>> Init on hart 0\n");
    printf(">>> BootROM DTB at %p\n", dtb);
    printf(">>> SPI controller setup\n");
    spi_init();
    printf(">>> Device startup\n");

    bring_all_adc(PATTERN); // start in test mode

    printf(">>> Accepting command from PCIe...\n");
  } else {
    while (true) wfi();
  }
  while (true) {
      int curr_cmd = recv_cmd();
      if (curr_cmd != cmd) {
          cmd = curr_cmd;

          if (cmd >= 0) {
              // bitslip calibration: low 2 bytes for bitslip, high 2 bytes for pattern
              uint16_t pattern = ((uint32_t)cmd & 0xffff0000) >> 16;
              bring_all_adc(pattern);

              uint16_t slip = cmd & 0xffff;
              if (slip >= 64) {
                  printf("Invalid bitslip %d: must be in [0, 64)\n", slip);
              } else {
                  bitslip = slip;
                  //printf("Updating bitslip = %d\n", bitslip);
                  write_gpio_reg(bitslip << 4);
              }
          } else {
              // normal operation
              bring_all_adc(-1);
          }
      }
  }
}
