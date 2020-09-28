// See LICENSE for license details.

#include "uart.h"

volatile uint32_t *uart_base_ptr = (uint32_t *)(UART_BASE);

void uart_init() {

  // Enable 8 byte FIFO
  *(uart_base_ptr + UART_FCR) = 0x0081;

  // set 0x0080 to UART.LCR to enable DLL and DLM write
  // configure baud rate
  *(uart_base_ptr + UART_LCR) = 0x0080;

  // System clock 100 MHz, 115200 baud rate
  // divisor = clk_freq / (16 * Baud)
  *(uart_base_ptr + UART_DLL) = 100 * 1000 * 1000u / (16u * 115200u) % 0x100u;
  *(uart_base_ptr + UART_DLM) = 100 * 1000 * 1000u / (16u * 115200u) >> 8;

  // 8-bit data, no parity
  *(uart_base_ptr + UART_LCR) = 0x0003u;

  // Enable read IRQ
  *(uart_base_ptr + UART_IER) = 0x0001u;

  // print "uart is working ..."
  // uart_demo();
}

void uart_send(uint8_t data) {
  // wait until THR empty
  while (!(*(uart_base_ptr + UART_LSR) & 0x40u))
    ;
  *(uart_base_ptr + UART_THR) = data;
}
