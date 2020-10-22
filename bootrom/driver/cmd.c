#include "cmd.h"

volatile uint32_t *cmd_base_ptr = (uint32_t*)(CMD_BASE);

void init_cmd() {
    *cmd_base_ptr = ~0;
}

int32_t recv_cmd() {
    return *cmd_base_ptr;
}
