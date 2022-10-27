#ifndef BUFFER_H
#define BUFFER_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct buffer_t {
    uint8_t index;
    uint8_t length;
    const uint8_t size;
    uint8_t data[]; // cppcheck-suppress [misra-c2012-18.7]
} buffer_t;

#define BUFFER(length)                                                                                                 \
    ((buffer_t *)&(union {                                                                                             \
        uint8_t data[sizeof(buffer_t) + (length)];                                                                     \
        buffer_t buffer;                                                                                               \
    }){.buffer.size = (length)}) // cppcheck-suppress [misra-c2012-2.5]

#define BUFFER_STATIC(name, length)                                                                                    \
    union(name)##_union {                                                                                              \
        uint8_t data[sizeof(buffer_t) + (length)];                                                                     \
        buffer_t buffer;                                                                                               \
    };                                                                                                                 \
    static union(name)##_union(name)##_buffer = {.buffer.size = (length)};                                             \
    static buffer_t *(name) = (buffer_t *)&(name)##_buffer // cppcheck-suppress [misra-c2012-20.10, misra-c2012-2.5]

static inline void buffer_reset(buffer_t *buf) {
    buf->index = 0;
    buf->length = 0;
}

static inline void buffer_put_byte(buffer_t *buf, uint8_t byte) {
    if (buf->length < buf->size) {
        buf->data[buf->length] = byte;
        buf->length++;
    }
}

static inline uint8_t buffer_get_byte(buffer_t *buf) {
    uint8_t byte = 0;
    if (buf->index < buf->length) {
        byte = buf->data[buf->index];
        buf->index++;
    }
    return byte;
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* BUFFER_H_ */
