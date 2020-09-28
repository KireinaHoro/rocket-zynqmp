#include "bits.h"

void main(int hartid, void *dtb) {
  if (hartid == 0) {
    uart_init();
    // trap requires UART
    setup_trap();
    printf(">>> Init on hart 0\n");
    printf(">>> BootROM DTB at %p\n", dtb);

    printf("TEST STARTUP - SPINNING\n");

    goto hang;
  } else {
    goto hang;
  }
hang:
  while (true) {
    wfi();
  }
}
