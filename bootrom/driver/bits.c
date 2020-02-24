#include "myprintf.h"
#include <stdbool.h>
#include <stdint.h>

void _assert(bool x, const char *file, int lineno) {
	if (!(x)) {
		printf("!!! Assertion failed at %s", (uint64_t)file);
		printf(":%d\r\n", __LINE__);
		for (;;);
	}
}
