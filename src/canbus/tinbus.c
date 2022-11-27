#include <assert.h>

#include "frame.h"

bool frame_get_bit(const tinbus_frame_t *frame, uint8_t index) {
    assert(index < TINBUS_BUFFER_SIZE * TINBUS_BITS_IN_BYTE);
    uint8_t bit_mask = 0b10000000u >> (index & 0b111u);
    uint8_t byte = index >> 3u;
    return frame->buffer[byte] & bit_mask;
}

char *tinbus_dump(const tinbus_frame_t *frame, char *dest) {
    char *out = dest;
    uint8_t bit = 0;
    while (bit < frame->bit_count) {
        if (frame_get_bit(frame, bit)) {
            *out++ = '1';
        } else {
            *out++ = '0';
        }
        bit++;
    }
    *out = '\0';
    return out;
}

// if (frame->bit_count == 0) {
//     string[0] = '\0';
//     return string;
// }
// char *out = string;
// uint8_t bits = frame->bit_count;
// uint8_t bytes = frame->bit_count >> 3u;

// uint8_t byte = 0;
// uint8_t bit_mask = 1 << frame->bit_count & 0b111;

// while (bits) {
//     bits--;
//     if (frame->data[byte] & (1 << bit)) {
//         *out++ = '1';
//     } else {
//         *out++ = '0';
//     }
// }

// while (i) {
//     if (i & code) {
//         *buf++ = '1';
//     } else {
//         *buf++ = '0';
//     }
//     i >>= 1;
// }
// *buf = '\0';
// return buf;
// }

// bool frame_push_bit(tinbus_frame_t *frame, bool bit){
//     if(frame->bit_count >= FRAME_BUFFER_SIZE << 3u){
//         return false;
//     }
//     frame->buffer[frame->bit_count >> 3u] |= 1 << (frame->bit_count & 3u);
//     frame->bit_count++;
//     return true;
// }
