#include "bits.h"

#define SEND(cc) \
{ \
    spi_select_slave(29 - i); \
    cmd[0] = ((cc) >> 16) & 0xff; \
    cmd[1] = (cc) & 0xff; \
    spi_send_multi(cmd, 4); \
    spi_deselect_slave(); \
    delay(500); \
}

// 0, 0.25, 1, 2, 4, 8, 16, 31.75
int attn_modes[] = {0x0, 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0xfe};

void setup_attn(int i, int mode) {
    uint8_t cmd[2];

    printf("Enabling attenuator %d with mode=%d...\n", i, mode);

    SEND(attn_modes[mode] << 8);
}

void bring_all_attns(int mode) {
    for (int i = 0; i < 2; ++i) {
        setup_attn(i, mode);
    }
}
