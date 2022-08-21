#include "cborm.h"

#define CBORM_TYPE_STRING (0x00U)
#define CBORM_TYPE_POSITIVE (0x40U)
#define CBORM_TYPE_NEGATIVE (0x80U)
#define CBORM_TYPE_BASE64 (0xC0U)

#define CBORM_TYPE_MASK (0xC0U)

#define CBORM_SIZE_1_BYTE (0x00U)
#define CBORM_SIZE_2_BYTE (0x10U)
#define CBORM_SIZE_3_BYTE (0x20U)
#define CBORM_SIZE_5_BYTE (0x30U)
#define CBORM_SIZE_MASK (0x30U)

bool cborm_write_base64(buffer_t *buf, uint8_t val) {
  bool is_ok = false;
  uint8_t value = val & ~CBORM_TYPE_MASK;
  if (buf->length <= (buf->size - 1U)) {
    buffer_put_byte(buf, CBORM_TYPE_BASE64 | value);
    is_ok = true;
  }
  return is_ok;
}

bool cborm_write_byte_array(buffer_t *buf, uint8_t *byte_array_in,
                            uint8_t size_in) {
  bool is_ok = false;
  uint8_t size = size_in & ~CBORM_TYPE_MASK;
  uint8_t *byte_array = byte_array_in;
  if (buf->length < (buf->size - size)) {
    buffer_put_byte(buf, CBORM_TYPE_STRING | size);
    while (size != 0U) {
      size--;
      buffer_put_byte(buf, *byte_array);
      byte_array++;
    }
    is_ok = true;
  }
  return is_ok;
}

bool cborm_write_integer(buffer_t *buf, int32_t val) {
  bool is_ok = false;
  uint32_t value = (uint32_t)val;
  uint8_t size = 0;
  uint8_t type = CBORM_TYPE_POSITIVE;

  if (val < 0) {
    uint32_t abs_value = -val;
    value = abs_value - 1U;
    type = CBORM_TYPE_NEGATIVE;
  }

  if (value < (1UL << 4U)) {
    size = 1;
  } else if (value < (1UL << 12U)) {
    size = 2;
  } else if (value < (1UL << 20U)) {
    size = 3;
  } else {
    size = 5;
  }

  if (buf->length < (buf->size - size)) {
    if (size == 1U) {
      buffer_put_byte(buf, type | CBORM_SIZE_1_BYTE | (uint8_t)value);
      is_ok = true;
    } else if (size == 2U) {
      buf->data[buf->length] =
          type | CBORM_SIZE_2_BYTE | (uint8_t)((value >> 8U) & 0x0FU);
      buf->length++;
      buffer_put_byte(buf, value);
      is_ok = true;
    } else if (size == 3U) {
      buf->data[buf->length] =
          type | CBORM_SIZE_3_BYTE | (uint8_t)((value >> 16U) & 0x0FU);
      buf->length++;
      buffer_put_byte(buf, value >> 8U);
      buffer_put_byte(buf, value);
      is_ok = true;
    } else {
      buffer_put_byte(buf, type | CBORM_SIZE_5_BYTE);
      buffer_put_byte(buf, value >> 24U);
      buffer_put_byte(buf, value >> 16U);
      buffer_put_byte(buf, value >> 8U);
      buffer_put_byte(buf, value);
      is_ok = true;
    }
  }
  return is_ok;
}

void cborm_read_init(buffer_t *buf) { buf->index = 0U; }

cborm_t cborm_read(buffer_t *buf) {
  cborm_t data = {.type = CBORM_DATA_ERROR};
  // data.type = CBORM_DATA_ERROR;

  if (buf->index < buf->length) {
    uint8_t first_byte = buffer_get_byte(buf);
    uint8_t type = first_byte & CBORM_TYPE_MASK;
    if (type == CBORM_TYPE_BASE64) {
      uint8_t value = first_byte & ~CBORM_TYPE_MASK;
      data.number = (int32_t)value;
      data.type = CBORM_DATA_BASE64;
    } else if (type == CBORM_TYPE_STRING) {
      data.array.data = &buf->data[buf->index];
      uint8_t size = first_byte & ~CBORM_TYPE_MASK;
      buf->index += size;
      data.array.size = size;
      data.type = CBORM_DATA_STRING;
    } else if ((type == CBORM_TYPE_POSITIVE) || (type == CBORM_TYPE_NEGATIVE)) {
      uint8_t size = first_byte & CBORM_SIZE_MASK;
      uint8_t value_msb = first_byte & (~(CBORM_TYPE_MASK | CBORM_SIZE_MASK));
      uint32_t value = (uint32_t)value_msb;
      data.type = CBORM_DATA_INTEGER;
      if (size == CBORM_SIZE_1_BYTE) {
        // data.type = CBORM_DATA_INTEGER;
      } else if (size == CBORM_SIZE_2_BYTE) {
        value <<= 8U;
        value |= (uint32_t)buffer_get_byte(buf);
      } else if (size == CBORM_SIZE_3_BYTE) {
        value <<= 8U;
        value |= (uint32_t)buffer_get_byte(buf);
        value <<= 8U;
        value |= (uint32_t)buffer_get_byte(buf);
      } else if (size == CBORM_SIZE_5_BYTE) {
        value = (uint32_t)buffer_get_byte(buf);
        value <<= 8U;
        value |= (uint32_t)buffer_get_byte(buf);
        value <<= 8U;
        value |= (uint32_t)buffer_get_byte(buf);
        value <<= 8U;
        value |= (uint32_t)buffer_get_byte(buf);
      } else {
        data.type = CBORM_DATA_ERROR;
      }
      if (type == CBORM_TYPE_NEGATIVE) {
        data.number = -1 - (int32_t)value;
      } else {
        data.number = (int32_t)value;
      }
    } else {
      data.type = CBORM_DATA_ERROR;
    }
  }
  return data;
}
