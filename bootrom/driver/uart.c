// See LICENSE for license details.

#include "uart.h"

volatile uint32_t *uart_base_ptr = (uint32_t *)(UART_BASE);

void uart_init() {
    *(uart_base_ptr + UART_LCR) = 0x03; // reset RX and TX FIFO
}

void uart_send(uint8_t data) {
  // wait until TX FIFO not full
  while (*(uart_base_ptr + UART_LSR) & 0x08u)
    ;
  *(uart_base_ptr + UART_TX) = data;
}

uint8_t uart_recv() {
  // wait for Rx FIFO valid data
  while (!(*(uart_base_ptr + UART_LSR) & 0x01u))
    ;
  return *(uart_base_ptr + UART_RX);
}
