
#include "myprintf.h"

void putch(uint8_t ch) { uart_send(ch); }

// void printf(char *s, ...)
void printf(char *s, uint64_t num) {
  int i = 0;
  // va_list va_ptr;
  // va_start(va_ptr, s);
  while (s[i] != '\0') {
    if (s[i] != '%')
      putch(s[i++]);
    else
      switch (s[++i]) {
      case 'd': {
        // printDeci(va_arg(va_ptr,int));
        if (num == 0)
          putch('0');
        else
          printNum(num, 10);
        i++;
        continue;
      }
      case 'p': {
        // printDeci(va_arg(va_ptr,int));
        putch('0');
        putch('x');
        if (num == 0)
          putch('0');
        else
          printNum(num, 16);
        i++;
        continue;
      }
      case 'x': {
        // printDeci(va_arg(va_ptr,int));
        if (num <= 0xf)
          putch('0');
        if (num == 0)
          putch('0');
        else
          printNum(num, 16);
        i++;
        continue;
      }
      case 's': {
        printf((char *)num, 0);
        i++;
        continue;
      }
      default: {
        i++;
        continue;
      }
      }
  }

  // va_end(va_ptr);
}

void printNum(uint64_t num, int base) {
  if (num == 0)
    return;
  printNum(num / base, base);
  putch("0123456789abcdef"[num % base]);
}

void hexdump(const void* data, size_t size) {
	char ascii[17];
	size_t i, j;
	ascii[16] = '\0';
	for (i = 0; i < size; ++i) {
		printf("%x ", ((unsigned char*)data)[i]);
		if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
			ascii[i % 16] = ((unsigned char*)data)[i];
		} else {
			ascii[i % 16] = '.';
		}
		if ((i+1) % 8 == 0 || i+1 == size) {
			printf(" ", 0);
			if ((i+1) % 16 == 0) {
				printf("|  %s \r\n", (uint64_t)ascii);
			} else if (i+1 == size) {
				ascii[(i+1) % 16] = '\0';
				if ((i+1) % 16 <= 8) {
					printf(" ", 0);
				}
				for (j = (i+1) % 16; j < 16; ++j) {
					printf("   ", 0);
				}
				printf("|  %s \r\n", (uint64_t)ascii);
			}
		}
	}
}