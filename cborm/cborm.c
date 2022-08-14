#include <stdbool.h>
#include <stdint.h>

#include <stdio.h>

#include "buffer.h"

#define CBORM_TYPE_ERROR (0xFF)
#define CBORM_TYPE_NONE (0xFE)

#define CBORM_TYPE_STRING (0x00)
#define CBORM_TYPE_POSITIVE (0x40)
#define CBORM_TYPE_NEGATIVE (0x80)
#define CBORM_TYPE_BASE64 (0xc0)
#define CBORM_TYPE_MASK (0xc0)

#define CBORM_SIZE_1_BYTE (0x00)
#define CBORM_SIZE_2_BYTE (0x10)
#define CBORM_SIZE_3_BYTE (0x20)
#define CBORM_SIZE_5_BYTE (0x30)
#define CBORM_SIZE_MASK (0x30)

typedef struct cborm_t {
  uint8_t type;
  union {
    int32_t number;
    struct byte_array {
      uint8_t *data;
      uint8_t size;
    } array;
  };
} cborm_t;

#define CBORM_BUF_SIZE (32)

static uint8_t bin2hex(uint8_t bin) {
  bin &= 0x0F;
  if (bin < 10) {
    return bin + '0';
  } else {
    return bin + 'A' - 10;
  }
}

static void hexdump(uint8_t buffer[], uint8_t size) {
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

bool cborm_write_base64(buffer_t *buf, uint8_t value) {
  value &= ~CBORM_TYPE_MASK;
  if (buf->length > buf->size - 1) {
    return false;
  } else {
    buf->data[buf->length++] = CBORM_TYPE_BASE64 | value;
  }
  return true;
}

bool cborm_write_integer(buffer_t *buf, int32_t value) {
  uint8_t size = 5;
  uint8_t type = CBORM_TYPE_POSITIVE;

  if (value < 0) {
    value = -1 - value;
    type = CBORM_TYPE_NEGATIVE;
  }

  if (value < (1 << 4)) {
    size = 1;
  } else if (value < (1 << 12)) {
    size = 2;
  } else if (value < (1 << 20)) {
    size = 3;
  }

  if (buf->length >= buf->size - size) {
    return false;
  }

  if (size == 1) {
    buf->data[buf->length++] = type | CBORM_SIZE_1_BYTE | (uint8_t)value;
  } else if (size == 2) {
    buf->data[buf->length++] =
        type | CBORM_SIZE_2_BYTE | (uint8_t)((value >> 8) & 0x0f);
    buf->data[buf->length++] = value;
  } else if (size == 3) {
    buf->data[buf->length++] =
        type | CBORM_SIZE_3_BYTE | (uint8_t)((value >> 16) & 0x0f);
    buf->data[buf->length++] = value >> 8;
    buf->data[buf->length++] = value;
  } else {
    buf->data[buf->length++] = type | CBORM_SIZE_5_BYTE;
    buf->data[buf->length++] = value >> 24;
    buf->data[buf->length++] = value >> 16;
    buf->data[buf->length++] = value >> 8;
    buf->data[buf->length++] = value;
  }

  return true;
}

bool cborm_write_byte_array(buffer_t *buf, uint8_t *byte_array, uint8_t size) {
  size &= ~CBORM_TYPE_MASK;
  if (buf->length >= buf->size - size) {
    return false;
  }

  buf->data[buf->length++] = CBORM_TYPE_STRING | size;
  while (size--) {
    buf->data[buf->length++] = *byte_array++;
  }

  return true;
}

void cborm_read_init(buffer_t *buf) { buf->index = 0; }

bool cborm_read(buffer_t *buf, cborm_t *data) {

  if (buf->index >= buf->length) {
    data->type = CBORM_TYPE_NONE;
    printf("No more data : %d\n", data->type);
    return false;
  }

  uint8_t first_byte = buf->data[buf->index++];
  data->type = first_byte & CBORM_TYPE_MASK;

  if (data->type == CBORM_TYPE_BASE64) {
    data->number = first_byte & ~CBORM_TYPE_MASK;
    return true;
  }

  if ((data->type == CBORM_TYPE_POSITIVE) ||
      (data->type == CBORM_TYPE_NEGATIVE)) {
    uint8_t size = first_byte & CBORM_SIZE_MASK;
    if (size == CBORM_SIZE_1_BYTE) {
      data->number = first_byte & (~(CBORM_TYPE_MASK | CBORM_SIZE_MASK));
    } else if (size == CBORM_SIZE_2_BYTE) {
      data->number =
          (uint32_t)(first_byte & (~(CBORM_TYPE_MASK | CBORM_SIZE_MASK)));
      data->number <<= 8;
      data->number |= (uint32_t)buf->data[buf->index++];
    } else if (size == CBORM_SIZE_3_BYTE) {
      data->number =
          (uint32_t)(first_byte & (~(CBORM_TYPE_MASK | CBORM_SIZE_MASK)));
      data->number <<= 8;
      data->number |= (uint32_t)buf->data[buf->index++];
      data->number <<= 8;
      data->number |= (uint32_t)buf->data[buf->index++];
    } else if (size == CBORM_SIZE_5_BYTE) {
      data->number = (uint32_t)buf->data[buf->index++];
      data->number <<= 8;
      data->number |= (uint32_t)buf->data[buf->index++];
      data->number <<= 8;
      data->number |= (uint32_t)buf->data[buf->index++];
      data->number <<= 8;
      data->number |= (uint32_t)buf->data[buf->index++];
    } else {
      data->type = CBORM_TYPE_ERROR;
      printf("Number error : %d\n", size);
      return false;
    }

    if (data->type == CBORM_TYPE_NEGATIVE) {
      data->number = -1 - data->number;
    }

    return true;
  }

  if (data->type == CBORM_TYPE_STRING) {
    data->array.data = &buf->data[buf->index];
    uint8_t size = first_byte & ~CBORM_TYPE_MASK;
    buf->index += size;
    data->array.size = size;
    return true;
  }

  data->type = CBORM_TYPE_ERROR;
  printf("Type error : %d\n", data->type);
  return false;
}

// BUFFER_STATIC(buffer_static, CBORM_BUF_SIZE);

int main() {

  buffer_t *buffer = BUFFER(CBORM_BUF_SIZE);
  // buffer_t *buffer = buffer_static;

  cborm_write_base64(buffer, 13);
  hexdump(buffer->data, buffer->size);
  cborm_write_integer(buffer, 12);
  hexdump(buffer->data, buffer->size);
  cborm_write_integer(buffer, -13000);
  hexdump(buffer->data, buffer->size);
  cborm_write_byte_array(buffer, (uint8_t *)"Hello", 6);
  hexdump(buffer->data, buffer->size);
  cborm_write_base64(buffer, 10);
  hexdump(buffer->data, buffer->size);
  cborm_write_integer(buffer, 1000000000);
  hexdump(buffer->data, buffer->size);


  cborm_t data;
  cborm_read_init(buffer);
  while (cborm_read(buffer, &data)) {
    printf("\nData type : %d\n", data.type);
    if (data.type == CBORM_TYPE_STRING) {
      printf("String : %s\n", data.array.data);
    } else if (data.type == CBORM_TYPE_BASE64) {
      printf("Base64 : %d\n", data.number);
    } else if (data.type == CBORM_TYPE_POSITIVE) {
      printf("Positive : %d\n", data.number);
    } else if (data.type == CBORM_TYPE_NEGATIVE) {
      printf("Negative : %d\n", data.number);
    }
  }

  return 0;
}