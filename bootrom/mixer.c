#include "bits.h"

#include "spi371109.h"

#define READBACK

#define SEND(cc) \
{ \
    trf371109_regrw(15 - i, cc); \
}

#ifdef READBACK
#define RECV(NAME, ADDR) \
{ \
    uint32_t readback = trf371109_regrw(15 - i, 0x10000001 | (ADDR << 1)); \
    DEBUG_P(#NAME " = %#x\n", readback); \
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

void setup_mixer(int i, uint8_t gain, uint8_t attn) {
#ifdef READBACK
    printf("Reading back regs for mixer %d...\n", i);
    RECV(R1, 0x4)
    RECV(R2, 0x2)
    RECV(R3, 0x6)
    RECV(R5, 0x5)
    RECV(R0, 0x0)
#endif

	gain = reverse(gain);

    //SEND(0x913f0080); // default value for Reg 1
    //SEND(0x953f0080); // Mixer power down = 1
	SEND(0x91300080 | gain << 15 | attn);
}

void bring_all_mixer(uint8_t gain, uint8_t attn) {
    printf(">>> Starting all mixers...\n");
    for (int i = 0; i < 16; ++i) {
        setup_mixer(i, gain, attn);
    }
}
