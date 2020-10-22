#include "bits.h"

void setup_adc(int i, long test_pattern) {
#ifdef READBACK
    uint8_t readback = 0;
#endif
    if (test_pattern > 0) {
        printf(">>> Bringing up ADC @ SC%c_%c with test pattern %#x...\n", 'A' + ((7 - i) / 2), 'A' + ((7 - i) % 2), (uint32_t)test_pattern);
    } else {
        printf(">>> Bringing up ADC @ SC%c_%c for real data...\n", 'A' + ((7 - i) / 2), 'A' + ((7 - i) % 2));
    }

    spi_select_slave(16 + i);
    spi_send(0x00 | 0x00); // write 00h: reset
    spi_send(0x80);
    spi_deselect_slave();

    // 00h reset is write-only

    if (test_pattern > 0) {
        spi_select_slave(16 + i);
        spi_send(0x00 | 0x03); // write 03h: pattern MSB
        spi_send(0x80 | ((test_pattern >> 8) & 0x3f)); // OUTTEST = 1
        spi_deselect_slave();

#ifdef READBACK
        spi_select_slave(16 + i);
        spi_send(0x80 | 0x03); // read 03h: pattern MSB
        spi_recv_multi(&readback, 1);
        spi_deselect_slave();
        DEBUG_P("03h = %#x\n", readback);
#endif

        spi_select_slave(16 + i);
        spi_send(0x00 | 0x04); // write 04h: pattern LSB
        spi_send(test_pattern & 0xff);
        spi_deselect_slave();

#ifdef READBACK
        spi_select_slave(16 + i);
        spi_send(0x80 | 0x04); // read 04h: pattern LSB
        spi_recv_multi(&readback, 1);
        spi_deselect_slave();
        DEBUG_P("04h = %#x\n", readback);
#endif
    } else {
        // no test pattern
        spi_select_slave(16 + i);
        spi_send(0x00 | 0x03); // write 03h: pattern MSB
        spi_send(0x00); // OUTTEST = 1
        spi_deselect_slave();

#ifdef READBACK
        spi_select_slave(16 + i);
        spi_send(0x80 | 0x03); // read 03h: pattern MSB
        spi_recv_multi(&readback, 1);
        spi_deselect_slave();
        DEBUG_P("03h = %#x\n", readback);
#endif
    }

    spi_select_slave(16 + i);
    spi_send(0x00 | 0x02); // write 02h: output mode
    spi_send(0x07);        // 3.5mA | no-termination | output-on | 1-lane 16bit
    spi_deselect_slave();

#ifdef READBACK
    spi_select_slave(16 + i);
    spi_send(0x80 | 0x02); // read 02h: output mode
    spi_recv_multi(&readback, 1);
    spi_deselect_slave();
    DEBUG_P("02h = %#x\n", readback);
#endif
}
