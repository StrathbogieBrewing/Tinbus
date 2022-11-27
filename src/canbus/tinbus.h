#ifndef TINBUS_H
#define TINBUS_H

#include <stdint.h>
#include <stdbool.h>

#define TINBUS_BUFFER_SIZE (15)
#define TINBUS_BITS_IN_BYTE (8)

typedef struct tinbus_t{
    uint8_t bit_count;
    uint8_t buffer[TINBUS_BUFFER_SIZE];
} tinbus_frame_t;

char *tinbus_dump(const tinbus_frame_t *frame, char *dest);


#endif // TINBUS_H