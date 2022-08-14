#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct buffer_t {
  uint8_t index;
  uint8_t length;
  uint8_t size;
  uint8_t data[];
} buffer_t;

#define BUFFER(length)                                                         \
  ((buffer_t *)&(union {                                                       \
    uint8_t data[sizeof(buffer_t) + length];                                   \
    buffer_t buffer;                                                           \
  }){.buffer.size = length})

#define BUFFER_STATIC(name, length)                                            \
  union name##_union {                                                         \
    uint8_t data[sizeof(buffer_t) + length];                                   \
    buffer_t buffer;                                                           \
  };                                                                           \
  static union name##_union name##_buffer = {.buffer.size = length};           \
  static buffer_t *name = (buffer_t *)&name##_buffer

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* BUFFER_H_ */
