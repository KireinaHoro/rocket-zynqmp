#include "bits.h"

#include "assert.h"

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

void setup_synth(int i, int freq, int chdiv) {
    uint8_t cmd[3];

#ifdef READBACK
    uint8_t readback[2];
#endif

    int vco = freq * chdiv;
    assert(vco > 3200 && vco < 6400);
    
    int pll_n = vco / 100;
    int pll_rem = vco % 100;
    uint32_t pll_dem = 10000000;
    uint32_t pll_num = pll_rem * pll_dem / 100;

    // reset registers
    SEND(0x00, 0x22, 0x1e)

    // enable readback, power down device
    SEND(0x00, 0x22, 0x19)

    // PLL_N
    SEND(0x24, (pll_n >> 8) & 0xff, pll_n & 0xff)
    RECV(PLL_N, 0x24)

    uint16_t pll_num_hi = (pll_num >> 16);
    // PLL_NUM[31:16]
    SEND(0x2a, (pll_num_hi >> 8) & 0xff, pll_num_hi & 0xff)
    RECV(PLL_NUM_HI, 0x2a)

    uint16_t pll_num_lo = pll_num & 0xffff;
    // PLL_NUM[15:0]
    SEND(0x2b, (pll_num_lo >> 8) & 0xff, pll_num_lo & 0xff)
    RECV(PLL_NUM_LO, 0x2b)

    uint16_t pll_dem_hi = (pll_dem >> 16);
    // PLL_NUM[31:16]
    SEND(0x26, (pll_dem_hi >> 8) & 0xff, pll_dem_hi & 0xff)
    RECV(PLL_DEM_HI, 0x26)

    uint16_t pll_dem_lo = pll_dem & 0xffff;
    // PLL_NUM[15:0]
    SEND(0x27, (pll_dem_lo >> 8) & 0xff, pll_dem_lo & 0xff)
    RECV(PLL_DEM_LO, 0x27)

    // CHDIV
    int count =
        chdiv == 2 ? 0 :
        chdiv == 4 ? 1 :
        chdiv == 8 ? 3 :
        chdiv == 16 ? 5 :
        chdiv == 32 ? 7 :
        chdiv == 64 ? 9 :
        chdiv == 128 ? 12 :
        chdiv == 256 ? 14 : -1;
    assert(count != -1);
    SEND(0x4b, 0x08 & (count >> 2), (count & 0x3) << 6)
    RECV(CHDIV, 0x4b)

    // power up RFout A and B
    SEND(0x2c, 0x08, 0x22)
    RECV(CH_POWER, 0x2c)

    // power up the device
    SEND(0x00, 0x22, 0x18)
    RECV(CFG, 0x00)
}

void bring_all_synth(int freq, int chdiv) {
    printf(">>> Starting all RF synthesizers with freq=%d chdiv=%d...\n", freq, chdiv);
    for (int i = 0; i < 2; ++i) {
        setup_synth(i, freq, chdiv);
    }
}
