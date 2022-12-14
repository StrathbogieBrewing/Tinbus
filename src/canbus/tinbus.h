#ifndef TINBUS_H
#define TINBUS_H

#include <stdint.h>
#include <stdbool.h>

#define TINBUS_BUFFER_SIZE (15)

typedef struct tinbus_t{
    uint8_t length;
    uint8_t buffer[TINBUS_BUFFER_SIZE];
} tinbus_frame_t;

typedef char tinbus_dump_t[TINBUS_BUFFER_SIZE * 8 + 1];

char *tinbus_dump(tinbus_dump_t dest, const tinbus_frame_t *frame);

void tinbus_open(void);
uint8_t tinbus_read(tinbus_frame_t *frame);
uint8_t tinbus_write(tinbus_frame_t *frame);
void tinbus_close(void);


#endif // TINBUS_H