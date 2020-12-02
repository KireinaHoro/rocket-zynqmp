#include "bits.h"

#define MIX_SCK 16
#define MIX_MOSI 15
#define MIX_MISO 0

//#define SSPI_DELAY() delay(50)
#define SSPI_DELAY()
#define WIO(id, val) set_bit((id), (val))
#define RIO(id) (get_bit((id)))

static inline void mix_cs_latch(int cs) {
	WIO(0x80000000 | cs, 1);
	SSPI_DELAY();
	WIO(MIX_SCK, 1);
	SSPI_DELAY();
	WIO(MIX_SCK, 0);
	WIO(0x80000000 | cs, 0); 
}
static uint32_t spi_rw(uint32_t data) {
	uint32_t out = 0;
	for (int i = 0; i < 32; i++) {
		WIO(MIX_MOSI, !!(data & (1UL << i)));
		SSPI_DELAY();
		WIO(MIX_SCK, 1);
		SSPI_DELAY();
		WIO(MIX_SCK, 0);
		out |= (int)!!RIO(MIX_MISO) << i;
	}
	return out;
}
uint32_t trf371109_regrw(int cs, uint32_t reg) {
	spi_rw(reg);
	mix_cs_latch(cs);
	return spi_rw(0);
}
