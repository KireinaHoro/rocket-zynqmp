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
    // default to 915 MHz
    setup_synth(0, 915, 4);
    bring_all_clkbuf();

    // default gain=15 attn=0
    bring_all_mixer(true, false, 15, 0, 128, 128);

    printf(">>> Enabling TX RF switches...\n");
    write_gpio_reg(0xf);

    printf(">>> Accepting command from PCIe...\n");
  } else {
    while (true)
      wfi();
  }
  while (true) {
    int curr_cmd = recv_cmd();
    if (curr_cmd != cmd) {
      cmd = curr_cmd;

      /* cmd[31:0] parse rule (MSB to LSB):
       *
       * opcode[1:0]:
       *    0: ADC bitslip calibration
       *    1: LO frequency
       *    2: mixer control
       *    3: enable ADC data sampling
       */

      switch (((uint32_t)cmd & 0xc0000000) >> 30) {
      case 0: {
        /* bitslip calibration
         * cmd[29:16]: pattern
         * cmd[15:0] : bitslip
         */
        uint16_t pattern = ((uint32_t)cmd & 0x3fff0000) >> 16;

        bring_all_adc(pattern);

        uint16_t slip = cmd & 0xffff;
        if (slip >= 64) {
          printf("Invalid bitslip %d: must be in [0, 64)\n", slip);
        } else {
          bitslip = slip;
          // printf("Updating bitslip = %d\n", bitslip);
          write_gpio_reg((bitslip << 4) | 0xf);
        }
        break;
      }
      case 1: {
        /* LO frequency
         * cmd[29:16]: synth 0 frequency in MHz
         * cmd[25:2] : synth 1 frequency in MHz
         * cmd[1]    : Clkbuf 0 (SCA SCB) use synth 0 or 1
         * cmd[0]    : Clkbuf 1 (SCA SCB) use synth 0 or 1
         */
        uint32_t synth0_freq = (cmd & 0x3fff0000) >> 16;
        uint32_t synth1_freq = (cmd & 0xfffc) >> 2;
        uint32_t clkbuf0 = (cmd & 0x2) >> 1;
        uint32_t clkbuf1 = cmd & 0x1;

        if (clkbuf0 == clkbuf1) {
          // start one synthesizer
          setup_synth(clkbuf0, clkbuf0 ? synth1_freq : synth0_freq, 4);
        } else {
          // start two synthesizers
          setup_synth(0, synth0_freq, 4);
          setup_synth(1, synth1_freq, 4);
        }

        // 1 is connected to clkin0 and vice versa
        setup_clkbuf(0, 1 - clkbuf0);
        setup_clkbuf(1, 1 - clkbuf1);

        break;
      }
      case 2: {
        /* mixer control
         * cmd[29:25]: gain [0, 24]
         * cmd[24]   : attenuator
         * cmd[23]   : enable
         * cmd[22]   : autocal
         * cmd[21:20]: reserved
         * cmd[19:12]: DC offset Q
         * cmd[11:4] : DC offset I
         * cmd[3:0]  : mixer ID
         */
        uint8_t gain = ((uint32_t)cmd & 0x3e000000) >> 25;
        uint8_t attn = ((uint32_t)cmd & 0x01000000) >> 24;
        uint8_t enable = ((uint32_t)cmd & 0x00800000) >> 23;
        uint8_t autocal = ((uint32_t)cmd & 0x00400000) >> 22;
        uint8_t dc_q = ((uint32_t)cmd & 0xff000) >> 12;
        uint8_t dc_i = ((uint32_t)cmd & 0xff0) >> 4;
        uint8_t id = ((uint32_t)cmd & 0xf);
        setup_mixer(id, enable, autocal, gain, attn, dc_i, dc_q);
        break;
      }
      case 3: {
        /* enable ADC data sampling
         * cmd[29:0]: reserved
         */
        write_gpio_reg((bitslip << 4) | 0xf);
        bring_all_adc(-1);
        break;
      }
      default: { printf("Unexpected command: %#x\n", cmd); }
      }
    }
  }
}
