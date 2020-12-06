#include "bits.h"

#define SEND(cc) \
{ \
    spi_select_slave(27 - i); \
    cmd[0] = ((cc) >> 24) & 0xff; \
    cmd[1] = ((cc) >> 16) & 0xff; \
    cmd[2] = ((cc) >> 8) & 0xff; \
    cmd[3] = (cc) & 0xff; \
    spi_send_multi(cmd, 4); \
    spi_deselect_slave(); \
}

void setup_clkbuf(int i, int clkin) {
    uint8_t cmd[4];

    printf("Enabling clkbuf %d with clkin=%d...\n", i, clkin);

    SEND(0x80000100);
    SEND(0x00010100);
    SEND(0x00010101);
    SEND(0x00010102);
    SEND(0x00010103);
    SEND(0x00010104);
    SEND(0x00010105);
    SEND(0x00010106);
    SEND(0x00010107);
    SEND(0x00032A09);
    if (clkin) {
        SEND(0x4800000E);
    } else {
        SEND(0x6800000E);
    }
}

void bring_all_clkbuf() {
    printf(">>> Starting all clock buffers...\n");
    for (int i = 0; i < 2; ++i) {
        // use clkin 1 as default
        setup_clkbuf(i, 1);
    }
}
