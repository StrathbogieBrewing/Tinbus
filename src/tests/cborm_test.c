#include <stdio.h>

#include "../cborm/cborm.h"
#include "../utils/utils.h"
#include "../buffer/buffer.h"

#define CBORM_TEST_BUF_SIZE (32)

int main() {

  buffer_t *buffer = BUFFER(CBORM_TEST_BUF_SIZE);
  // buffer_t *buffer = buffer_static;

  cborm_write_base64(buffer, 13);
  hexdump(buffer->data, buffer->length);
  cborm_write_integer(buffer, 12);
  hexdump(buffer->data, buffer->length);
  cborm_write_integer(buffer, -13000);
  hexdump(buffer->data, buffer->length);
  cborm_write_byte_array(buffer, (uint8_t *)"Hello", 6);
  hexdump(buffer->data, buffer->length);
  cborm_write_base64(buffer, 10);
  hexdump(buffer->data, buffer->length);
  cborm_write_integer(buffer, 1000000000);
  hexdump(buffer->data, buffer->length);

  cborm_t data;
  cborm_read_init(buffer);
  while ((data = cborm_read(buffer)).type != CBORM_DATA_ERROR) {
    printf("\nData type : %d\n", data.type);
    if (data.type == CBORM_DATA_STRING) {
      printf("String : %s\n", data.array.data);
    } else if (data.type == CBORM_DATA_BASE64) {
      printf("Base64 : %d\n", data.number);
    } else if (data.type == CBORM_DATA_INTEGER) {
      printf("Positive : %d\n", data.number);
    } else {
      printf("Data type not found : %d\n", data.type);
    }
  }

  return 0;
}