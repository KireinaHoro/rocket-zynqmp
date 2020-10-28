#include "bits.h"

#define READBACK

#define SEND(B_0, B_1, B_2) \
{ \
    spi_select_slave(25 - i); \
    cmd[0] = B_0; \
    cmd[1] = B_1; \
    cmd[2] = B_2; \
    spi_send_multi(cmd, 3); \
    spi_deselect_slave(); \
}

#ifdef READBACK
#define RECV(NAME, ADDR) \
{ \
    spi_select_slave(25 - i); \
    spi_send((ADDR) | 0x80); \
    spi_recv_multi(readback, 2); \
    spi_deselect_slave(); \
    DEBUG_P(#NAME " = %#x %#x\n", readback[0], readback[1]); \
}
#else
#define RECV(NAME, ADDR)
#endif

void setup_synth(int i) {
    uint8_t cmd[3];

#ifdef READBACK
    uint8_t readback[2];
#endif

    // reset registers
    SEND(0x00, 0x22, 0x1e)

    // enable readback, power down device
    SEND(0x00, 0x22, 0x19)

    // PLL_N = 73
    SEND(0x24, 0x00, 0x49)
    RECV(PLL_N, 0x24)

    // PLL_NUM[31:16] = 6103
    SEND(0x2a, 0x17, 0xd7)
    RECV(PLL_NUM_HI, 0x2a)

    // PLL_NUM[15:0] = 33792
    SEND(0x2b, 0x84, 0x00)
    RECV(PLL_NUM_LO, 0x2b)

    // PLL_DEN[31:16] = 30517
    SEND(0x26, 0x77, 0x35)
    RECV(PLL_DEN_HI, 0x26)

    // PLL_DEN[15:0] = 37888
    SEND(0x27, 0x94, 0x00)
    RECV(PLL_DEN_LO, 0x27)

    // CHDIV = 8
    SEND(0x4b, 0x08, 0xc0)
    RECV(CHDIV, 0x4b)

    // power up RFout A and B
    SEND(0x2c, 0x08, 0x22)
    RECV(CH_POWER, 0x2c)

    // power up the device
    SEND(0x00, 0x22, 0x18)
    RECV(CFG, 0x00)
}

void bring_all_synth() {
    printf(">>> Starting all RF synthesizers...\n");
    for (int i = 0; i < 2; ++i) {
        setup_synth(i);
    }
}
