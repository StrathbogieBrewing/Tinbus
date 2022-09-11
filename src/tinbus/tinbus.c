
#include "buffer.h"

#define TINBUS_DONE (0)

#define TINBUS_ZERO_CHAR (0x7F)
#define TINBUS_ONE_CHAR (0xFF)

#define TINBUS_READY (0)
#define TINBUS_BUSY (1)
#define TINBUS_ERROR (2)

typedef struct tinbus_t {
    buffer_t *buffer;
    uint8_t bit_counter;
    uint8_t shift_register;
    uint8_t status;
} tinbus_t;

void tinbus_init(tinbus_t *frame) {
    frame->bit_counter = 0;
    frame->shift_register = 0xFF;
    buffer_reset(frame->buffer);
}

void tinbus_put_bit(tinbus_t *frame, bool bit) {
    frame->status = TINBUS_BUSY;
    /* End of frame encoded with 0b000000001 */
    if ((frame->shift_register == 0x00) && bit) {
        /* End of frame */
        frame->shift_register = 0xFF;
        if (frame->bit_counter != 0) {
            frame->bit_counter = 0;
            frame->status = TINBUS_ERROR;
        } else {
            frame->status = TINBUS_READY;
        }
    } else {
        frame->shift_register <<= 1;
        frame->bit_counter++;
        if (bit) {
            frame->shift_register |= 0x01;
        }
        if (frame->bit_counter == 8) {
            frame->bit_counter = 0;
            buffer_put_byte(frame->buffer, frame->shift_register);
        }
    }
}

bool tinbus_get_bit(tinbus_t *frame) {
    frame->status = TINBUS_BUSY;
    if (frame->bit_counter == 0) {
        frame->bit_counter = 8;
        frame->shift_register = buffer_get_byte(frame->buffer);
    }

    bool bit = frame->shift_register & 0x80;
    frame->shift_register <<= 1;
    frame->bit_counter--;

    return bit;
}