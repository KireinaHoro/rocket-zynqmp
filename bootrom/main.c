#include "bits.h"

#define DEBUG_P(...) printf(__VA_ARGS__)
//#define DEBUG_P(...)

//#define READBACK

void main(int hartid, void *dtb) {
  int bitslip = 0;
  init_cmd();
  uint32_t cmd = recv_cmd();
  if (hartid == 0) {
    uart_init();
    // trap requires UART
    setup_trap();
    printf("\n>>> Init on hart 0\n");
    printf(">>> BootROM DTB at %p\n", dtb);
    printf(">>> SPI controller setup\n");
    spi_init();
    printf(">>> Device startup\n");

    printf(">>> Enabling ADC clock...\n");
    write_gpio_reg(0x10); // 0x10: EN_CLK_ADC

    for (int i = 7; i >= 0; --i) {
#ifdef READBACK
        uint8_t readback = 0;
#endif

        printf(">>> Bringing up ADC @ SC%c_%c...\n", 'A' + ((7 - i) / 2), 'A' + ((7 - i) % 2));

        spi_select_slave(16 + i);
        spi_send(0x00 | 0x00); // write 00h: reset
        spi_send(0x80);
        spi_deselect_slave();

        // 00h reset is write-only

        spi_select_slave(16 + i);
        spi_send(0x00 | 0x03); // write 03h: pattern MSB
        spi_send(0x80 | 0x19); // OUTTEST = 1
        spi_deselect_slave();

#ifdef READBACK
        spi_select_slave(16 + i);
        spi_send(0x80 | 0x03); // read 03h: pattern MSB
        spi_recv_multi(&readback, 1);
        spi_deselect_slave();
        DEBUG_P("03h = %#x\n", readback);
#endif

        spi_select_slave(16 + i);
        spi_send(0x00 | 0x04); // write 04h: pattern LSB
        spi_send(0x84);
        spi_deselect_slave();

#ifdef READBACK
        spi_select_slave(16 + i);
        spi_send(0x80 | 0x04); // read 04h: pattern LSB
        spi_recv_multi(&readback, 1);
        spi_deselect_slave();
        DEBUG_P("04h = %#x\n", readback);
#endif

        spi_select_slave(16 + i);
        spi_send(0x00 | 0x02); // write 02h: output mode
        spi_send(0x07);        // 3.5mA | no-termination | output-on | 1-lane 16bit
        spi_deselect_slave();

#ifdef READBACK
        spi_select_slave(16 + i);
        spi_send(0x80 | 0x02); // read 02h: output mode
        spi_recv_multi(&readback, 1);
        spi_deselect_slave();
        DEBUG_P("02h = %#x\n", readback);
#endif
    }

    printf(">>> Waiting for command from PCIe...\n");
  } else {
    while (true) wfi();
  }
  while (true) {
      uint32_t curr_cmd = recv_cmd();
      if (curr_cmd != cmd) {
          printf("New cmd: %#x\n", curr_cmd);
          cmd = curr_cmd;
          if (cmd >= 8) {
              printf("Invalid bitslip %d: must be in [0,8)\n", cmd);
          } else {
              bitslip = cmd;
              printf("Updating bitslip = %d\n", bitslip);
              write_gpio_reg(bitslip << 5 | 0x10); // 0x10: EN_CLK_ADC
          }
      }
  }
}
