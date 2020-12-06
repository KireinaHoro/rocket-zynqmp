#include "bits.h"
#include <assert.h>

void bit_diag(unsigned val) {
	for (int i = 0; i < 32; ++i) {
		printf("\tBit HOST#%2d, MIX#%2d = %1d\n", 31 - i, i, (val >> (31 - i)) & 1);
	}
}
static inline void __send(unsigned i, unsigned cc) { 
//#define MIX_VERBOSE
#ifdef MIX_VERBOSE
	printf("SENDing to device %d {\n", (i)); 
	bit_diag(cc);
	printf("}\n\n"); 
#endif
	spi_select_slave(15 - i); 
	uint8_t cmd[4];
	cmd[0] = ((cc) >> 24) & 0xff; 
	cmd[1] = ((cc) >> 16) & 0xff; 
	cmd[2] = ((cc) >> 8) & 0xff; 
	cmd[3] = (cc) & 0xff; 
	spi_send_multi(cmd, 4); 
	spi_deselect_slave(); 
	delay(500); 
} 

#define P2MASK(i) ((1u << (i)) - 1)
#define PH(len, last) (P2MASK(len) << (31 - (last))) //all-1 pattern placeholder for a register fieid
enum miXer_registeRs{
	XR1 = 0b10010 << (31 - 4) | (0b1 << (31 - 11)), 
	XR1_PWD_RF = PH(1, 5), 
	XR1_PWD_BUF = PH(1, 7), 
	XR1_PWD_FILT = PH(1, 8), 
	XR1_PWD_DC_OFF_DIG = PH(1, 10), 
	XR1_BBGAIN = PH(5, 16),
	XR1_LPFADJ = PH(8, 24), 
	XR1_EN_FLT_B = PH (2, 26), 
	XR1_EN_FASTGAIN = PH(1, 27), 
	XR1_EN_GAIN_SEL = PH(1, 28), 
	XR1_EN_OSC_TEST = PH(1, 29), 
	XR1_EN_3dB_ATTN = PH(1, 31),

	XR2 = 0b01010 << (31 - 4),
	XR2_EN_AUTOCAL = PH(1, 5),
	XR2_IDAC_BIT = PH(8, 13), 
	XR2_QDAC_BIT = PH(8, 21), 
	XR2_IDET_B = PH(2, 23), 
	XR2_CAL_SEL = PH(1, 24), 
	XR2_CLK_DIV_RATIO = PH(3, 27), 
	XR2_CAL_CLK_SEL = PH(1, 28),
	XR2_OSC_TRIM = PH(3, 31),
	
	XR3 = 0b11010 << (31 - 4),
	XR3_ILOAD_A = PH(6, 10),
	XR3_ILOAD_B = PH(6, 16),
	XR3_QLOAD_A = PH(6, 22),
	XR3_QLOAD_B = PH(6, 28),
	XR3_BYPASS = PH(1, 29),
	XR3_FLTR_CTRL_B = PH(2, 31),
	
	XR5 = 0b10110 << (31 - 4),
	XR5_MIX_GM_TRIM = PH(2, 6),
	XR5_VCM_TRIM = PH(2, 8),
	XR5_LO_TRIM = PH(2, 10),
	XR5_MIX_BUFF_TRIM = PH(2,12),
	XR5_FLTR_TRIM = PH(2, 14),
	XR5_OUT_BUFF_TRIM = PH(2, 16),

};
static inline unsigned u32_rev(unsigned a) {
	a = ((a >> 1) & 0x55555555) | ((a & 0x55555555) << 1);
	a = ((a >> 2) & 0x33333333) | ((a & 0x33333333) << 2);
	a = ((a >> 4) & 0x0F0F0F0F) | ((a & 0x0F0F0F0F) << 4);
	a = ((a >> 8) & 0x00FF00FF) | ((a & 0x00FF00FF) << 8);
	a = ( a >> 16             ) | ( a               << 16);
	return a;
}

static inline unsigned reg_rev(unsigned reg, unsigned val) {
	//shifts right so that have same numbers of leading zeros as register mask
	unsigned val_revsh = u32_rev(val) >> __builtin_clz(reg); 
	assert(0 == (val_revsh & ~reg)); 
	return val_revsh; 
}


void setup_mixer(int i, bool enable, bool autocal, uint8_t gain, uint8_t attn, uint32_t dc_i, uint32_t dc_q) {

#define SEND(val) __send(i, (val))
	//printf("setup_mixer i=%d enable=%d autocal=%d gain=%d attn=%d dc_i=%d dc_q=%d\n", i, enable, autocal, gain, attn, dc_i, dc_q);

	SEND(XR1 
			| reg_rev(XR1_PWD_RF, !enable)
			| reg_rev(XR1_PWD_BUF, 1)
			| reg_rev(XR1_PWD_FILT, 0)
			| reg_rev(XR1_PWD_DC_OFF_DIG, 0)
			| reg_rev(XR1_BBGAIN, gain) 
			| reg_rev(XR1_LPFADJ, 128u) 
			| reg_rev(XR1_EN_FLT_B, 3u)
			| reg_rev(XR1_EN_FASTGAIN, 0)
			| reg_rev(XR1_EN_GAIN_SEL, 0)
			| reg_rev(XR1_EN_OSC_TEST, 0)
			| reg_rev(XR1_EN_3dB_ATTN, attn) );
	
	if (autocal) {
		// register 2 - enable DC auto calibration
		//SEND(0x5004078e | 0x04000000);
		SEND(XR2 
				| reg_rev(XR2_EN_AUTOCAL, 1)
				| reg_rev(XR2_IDET_B, 3)
				| reg_rev(XR2_CAL_SEL, 1)
				| reg_rev(XR2_CLK_DIV_RATIO, 0)
				| reg_rev(XR2_CAL_CLK_SEL, 1)
				| reg_rev(XR2_OSC_TRIM, 0b011));
	} else {
		SEND(XR2
				| reg_rev(XR2_IDAC_BIT, dc_i)
				| reg_rev(XR2_QDAC_BIT, dc_q)
				| reg_rev(XR2_IDET_B, 3)
				| reg_rev(XR2_CAL_SEL, 0));
	}
#undef SEND
}

void bring_all_mixer(bool enable, bool autocal, uint8_t gain, uint8_t attn, uint32_t dc_i, uint32_t dc_q) {
	printf(">>> Starting all mixers...\n");
	for (int i = 0; i < 16; ++i) {
		setup_mixer(i, enable, autocal, gain, attn, dc_i, dc_q);
	}
}
