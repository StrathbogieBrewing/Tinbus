#include "tinbus.h"

char * tinbus_dump_binary(char *dest, uint8_t value){
    uint8_t bit_mask = 0b10000000;
    while (bit_mask){
        if(bit_mask & value){
            *dest = '1';
        } else {
            *dest = '0';
        }
        dest++;
        bit_mask >>= 1;
    }
    *dest = ' ';
    dest++;
    return dest;
}

char *tinbus_dump(char *dest, const tinbus_frame_t *frame) {
    uint8_t index = 0;
    if(frame->length > TINBUS_BUFFER_SIZE) return 0;
    while (index < frame->length) {
        dest = tinbus_dump_binary(dest, frame->buffer[index]);
        index++;
    }
    *dest = '\0';
    return dest;
}

