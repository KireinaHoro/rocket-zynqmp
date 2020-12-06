#include "bits.h"

#define SEND(cc) \
{ \
    spi_select_slave(29 - i); \
    cmd[0] = ((cc) >> 8) & 0xff; \
    cmd[1] = (cc) & 0xff; \
    spi_send_multi(cmd, 2); \
    spi_deselect_slave(); \
    delay(500); \
}

void setup_attn(int i, uint8_t attn) {
    uint8_t cmd[2];

    uint8_t addr;

    if (i % 2 == 0) {
        addr = 0;
    } else {
        addr = 0x80;
    }
    i /= 2;

    //printf("Enabling attenuator %d with attn=%#x...\n", i, attn);

    SEND(((uint32_t)attn << 8) | addr);
}

void bring_all_attns(uint8_t attn) {
    for (int i = 0; i < 4; ++i) {
        setup_attn(i, attn);
    }
}
