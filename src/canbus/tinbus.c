#include <assert.h>

#include "tinbus.h"

bool tinbus_get_bit(const tinbus_frame_t *frame, uint8_t index) {
    assert(index < frame->bit_count);
    uint8_t byte = index >> 3u;
    uint8_t bit_mask = 0b10000000u >> (index & 0b111u);
    return frame->buffer[byte] & bit_mask;
}

char *tinbus_dump(const tinbus_frame_t *frame, char *dest) {
    char *out = dest;
    uint8_t bit = 0;
    while (bit < frame->bit_count) {
        if (tinbus_get_bit(frame, bit)) {
            *out++ = '1';
        } else {
            *out++ = '0';
        }
        bit++;
    }
    *out = '\0';
    return out;
}

