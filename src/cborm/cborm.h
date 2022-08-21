#ifndef CBORM_H
#define CBORM_H

#include <stdbool.h>
#include <stdint.h>

#include "../buffer/buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  CBORM_DATA_ERROR = 0,
  CBORM_DATA_STRING,
  CBORM_DATA_INTEGER,
  CBORM_DATA_BASE64,
} cborm_data_type;

typedef struct cborm_t {
  cborm_data_type type;
  union { // cppcheck-suppress [misra-c2012-19.2]
    int32_t number;
    struct byte_array {
      uint8_t *data;
      uint8_t size;
    } array;
  };
} cborm_t;

bool cborm_write_base64(buffer_t *buf, uint8_t val);
bool cborm_write_integer(buffer_t *buf, int32_t val);
bool cborm_write_byte_array(buffer_t *buf, uint8_t *byte_array, uint8_t size);
void cborm_read_init(buffer_t *buf);
cborm_t cborm_read(buffer_t *buf);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* CBORM_H */
