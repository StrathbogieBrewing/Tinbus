#include <stdio.h>

#include "utils.h"

static uint8_t bin2hex(uint8_t bin) {
  bin &= 0x0F;
  if (bin < 10) {
    return bin + '0';
  } else {
    return bin + 'A' - 10;
  }
}

void hexdump(uint8_t buffer[], uint8_t size) {
  uint8_t index = 0;
  putchar(bin2hex(size >> 4));
  putchar(bin2hex(size));
  putchar('\t');
  while (index < size) {
    putchar(bin2hex(buffer[index] >> 4));
    putchar(bin2hex(buffer[index++]));
    putchar(' ');
  }
  putchar('\n');
}