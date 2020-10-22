#include "bits.h"

#define PATTERN 0x2a0c

#define DEBUG_P(...) printf(__VA_ARGS__)
//#define DEBUG_P(...)

void bring_all(long test_pattern) {
    static long current = -1;
    if (current != test_pattern) {
        for (int i = 7; i >= 0; --i) {
            setup_adc(i, test_pattern);
        }
        current = test_pattern;
    }
}

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

    bring_all(PATTERN); // start in test mode

    printf(">>> Enabling ADC clock...\n");
    write_gpio_reg(0x10); // 0x10: EN_CLK_ADC

    printf(">>> Waiting for command from PCIe...\n");
  } else {
    while (true) wfi();
  }
  while (true) {
      int curr_cmd = recv_cmd();
      if (curr_cmd != cmd) {
          printf("New cmd: %#x\n", curr_cmd);
          cmd = curr_cmd;

          if (cmd >= 0) {
              // bitslip calibration
              bring_all(PATTERN);
              if (cmd >= 8) {
                  printf("Invalid bitslip %d: must be in [0,8)\n", cmd);
              } else {
                  bitslip = cmd;
                  printf("Updating bitslip = %d\n", bitslip);
                  write_gpio_reg(bitslip << 5 | 0x10); // 0x10: EN_CLK_ADC
              }
          } else {
              // normal operation
              bring_all(-1);
          }
      }
  }
}
