#include "bits.h"

void main(int hartid, void *dtb) {
  if (hartid == 0) {
    uart_init();
    printf(">>> Init on hart 0\n");
    printf(">>> BootROM DTB at %p\n", dtb);
    printf(">>> Setting up machine mode trap...");
    fflush(stdout);
    setup_trap();
    printf("done.\n");

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
