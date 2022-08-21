#include "cobsm.h"

uint8_t cobsm_encode(uint8_t buffer[], uint8_t size) {
  uint8_t code = 0;
  uint8_t index = 0;
  uint8_t code_index = 0;
  buffer[size] = buffer[index];
  index++;
  while (index <= size) {
    if (buffer[index] == 0U) {
      buffer[code_index] = code + 1U;
      code_index = index;
      code = 0;
    } else {
      code++;
    }
    index++;
  }
  if (buffer[index - 1U] > code) {
    --index;
    buffer[code_index] = buffer[index];
  } else {
    buffer[code_index] = code + 1U;
  }
  return index;
}

uint8_t cobsm_decode(uint8_t *buffer, uint8_t size_in) {
  uint8_t size = size_in;
  uint8_t index = 0;
  uint8_t code_index = buffer[0];
  uint8_t code = buffer[code_index];
  if (code_index <= size) {
    while (index < size) {
      if (index == code_index) {
        code = buffer[index];
        buffer[index] = 0;
        code_index = index + code;
      }
      index++;
    }
    if (code_index <= size) {
      size--;
    }
    buffer[0] = buffer[size];
  }
  return size;
}