#include "bits.h"

#define PATTERN 0x2a0c

void main(int hartid, void *dtb) {
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
    write_gpio_field(0xf, 0, 4, 0);

    bring_all_attns(0xfe);

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
       *    3: ADC real data, attenuator
       */

      switch (((uint32_t)cmd & 0xc0000000) >> 30) {
      case 0: {
        /* bitslip calibration
         * cmd[29:16]: pattern
         * cmd[15:14]: mode
         *     00: reserved
         *     01: set bitslip only
         *     10: set bypass only
         *     11: set bypass + bitslip
         * cmd[13:11]: bypass_id
         * cmd[10:6] : bitslip_id
         * cmd[5:0]  : bitslip
         */
        uint16_t pattern = ((uint32_t)cmd & 0x3fff0000) >> 16;
        uint16_t mode = ((uint32_t)cmd & 0xc000) >> 14;
        uint16_t bypass_id = ((uint32_t)cmd & 0x3800) >> 11;
        uint16_t bitslip_id = ((uint32_t)cmd & 0x7c0) >> 6;
        uint16_t bitslip = ((uint32_t)cmd & 0x3f);

        //printf("cmd=%#x pattern=%#x mode=%#x bypass_id=%#x bitslip_id=%#x bitslip=%#x\n", cmd, pattern, mode, bypass_id, bitslip_id, bitslip);

        bring_all_adc(pattern);

        if (mode & 0x1) {
            write_gpio_field(bitslip_id, 15, 5, 0); // bitslip_id at [19:15]
            write_gpio_field(bitslip, 4, 6, 0);     // bitslip    at [9:4]
        }
        if (mode & 0x2) {
            write_gpio_field(bypass_id, 20, 3, 0);  // bypass_id  at [22:20]
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

#define synth_adiv(src, freq) setup_synth((src), (freq), (freq) <= 800 ? 8 : 4)
        if (clkbuf0 == clkbuf1) {
          // start one synthesizer
          synth_adiv(clkbuf0, clkbuf0 ? synth1_freq : synth0_freq);
        } else {
          // start two synthesizers
          synth_adiv(0, synth0_freq);
          synth_adiv(1, synth1_freq);
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
        /* enable ADC data sampling + attenuator
         * cmd[29]   : enable 
         * cmd[28:22]: atten 0
         * cmd[21:15]: atten 1
         * cmd[14:8] : atten 2
         * cmd[7:1]  : atten 3
         */
        uint8_t adc_enable = ((uint32_t)cmd & 0x20000000) >> 29;
        uint8_t attn0 = ((uint32_t)cmd & 0x1fc00000) >> 22;
        uint8_t attn1 = ((uint32_t)cmd & 0x003f8000) >> 15;
        uint8_t attn2 = ((uint32_t)cmd & 0x00007f00) >> 8;
        uint8_t attn3 = ((uint32_t)cmd & 0x000000fe) >> 1;

        if (adc_enable) {
            write_gpio_field(0xf, 0, 4, 0);
            bring_all_adc(-1);
        }
        setup_attn(0, attn0 << 1);
        setup_attn(1, attn1 << 1);
        setup_attn(2, attn2 << 1);
        setup_attn(3, attn3 << 1);
        break;
      }
      default: { printf("Unexpected command: %#x\n", cmd); }
      }
    }
  }
}
