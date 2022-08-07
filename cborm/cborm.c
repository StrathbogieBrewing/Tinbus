#include <stdint.h>

uint8_t* cborm_write_base64(uint8_t *dst, uint8_t value){

}

uint8_t* cborm_write_integer(uint8_t *dst, int32_t value){

}

uint8_t* cborm_write_bytearray(uint8_t *dst, uint8_t *buffer, uint8_t size){

}

typedef struct buffer_t {
    uint8_t *data;
    uint8_t index;
    uint8_t size;
    uint8_t max_size;
} buffer_t;

typedef struct cborm_t {
    uint8_t type;
    uint8_t base64;
    int32_t integer;
    buffer_t bytearray;
} cborm_t;

uint8_t* cborm_read(uint8_t *src, cborm_t data){

}

