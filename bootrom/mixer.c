#include "bits.h"

//#define READBACK

#define SEND(cc) \
{ \
    spi_select_slave(15 - i); \
    cmd[0] = (cc >> 24) & 0xff; \
    cmd[1] = (cc >> 16) & 0xff; \
    cmd[2] = (cc >> 8) & 0xff; \
    cmd[3] = (cc) & 0xff; \
    spi_send_multi(cmd, 4); \
    spi_deselect_slave(); \
    delay(500); \
}

#ifdef READBACK
#define RECV(NAME, ADDR) \
{ \
    spi_select_slave(15 - i); \
    cmd[0] = (0x10) & 0xff; \
    cmd[1] = (0x00) & 0xff; \
    cmd[2] = (0x00) & 0xff; \
    cmd[3] = ((ADDR) << 1 | 0x1) & 0xff; \
    spi_send_multi(cmd, 4); \
    spi_deselect_slave(); \
    spi_select_slave(15 - i); \
    spi_recv_multi(readback, 4); \
    spi_deselect_slave(); \
    delay(500); \
    DEBUG_P(#NAME " = %#x %#x %#x %#x\n", readback[0], readback[1], readback[2], readback[3]); \
}
#else
#define RECV(NAME, ADDR)
#endif

static unsigned char lookup[16] = {
0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf, };

uint8_t reverse(uint8_t n) {
   // Reverse the top and bottom nibble then swap them.
   return (lookup[n&0b1111] << 4) | lookup[n>>4];
}

void setup_mixer(int i, bool enable, bool autocal, uint8_t gain, uint8_t attn) {
    uint8_t cmd[4];

#ifdef READBACK
    uint8_t readback[4];
    RECV(R1, 0x4)
    RECV(R2, 0x2)
    RECV(R3, 0x6)
    RECV(R5, 0x5)
    RECV(R0, 0x0)
#endif

	gain = reverse(gain);

    if (!enable) {
        SEND(0x953f0080); // Mixer power down = 1
        return;
    } else {
        SEND(0x913f0080); // Mixer power down = 1
        return;
    }

    // register 1 - enable DC calibration
	SEND(0x91100080 | gain << 15 | attn | 0x60);

    // register 2 - disable DC auto calibration, Cal to 0xff IQ
    SEND(0x5004070e | (0xffff) << 10);

    return;
    if (autocal) {
        // register 2 - disable DC auto calibration
        SEND(0x5004078e | 0x04000000);
    }
}

void bring_all_mixer(bool enable, bool autocal, uint8_t gain, uint8_t attn) {
    printf(">>> Starting all mixers...\n");
    for (int i = 0; i < 16; ++i) {
        setup_mixer(i, enable, autocal, gain, attn);
    }
}
