#include <stdint.h>
#include <stdbool.h>

#define CBORM_TYPE_ERROR      (-1)
#define CBORM_TYPE_NONE      (0)
#define CBORM_TYPE_STRING    (0b00000000)
#define CBORM_TYPE_POSITIVE  (0b01000000)
#define CBORM_TYPE_NEGATIVE  (0b10000000)
#define CBORM_TYPE_BASE64    (0b11000000)
#define CBORM_TYPE_MASK      (0b11000000)

#define CBORM_SIZE_1_BYTE    (0b00000000)
#define CBORM_SIZE_2_BYTE    (0b00010000)
#define CBORM_SIZE_3_BYTE    (0b00100000)
#define CBORM_SIZE_5_BYTE    (0b00110000)
#define CBORM_SIZE_MASK      (0b00110000)



// #define CBORM_BUF(name, size) do{uint8_t data[size]; cborm_t name = {.data = data, .max_size = size};}

#define CBORM_BUF_SIZE (32)

// uint8_t data[CBORM_BUF_SIZE];


typedef struct cborm_t {
    uint8_t size;
    uint8_t index;
    const uint8_t max_size;
    uint8_t data[CBORM_BUF_SIZE];
} cborm_buf_t;

bool cborm_write_base64(cborm_buf_t *buf, uint8_t value){
    value &= ~CBORM_TYPE_MASK;
    if(buf->size > buf->max_size - 1){
        return false;
    } else {
        buf->data[buf->size++] = CBORM_TYPE_BASE64 | value;
    }
    return true;
}

bool cborm_write_integer(cborm_buf_t *buf, int32_t value){
    uint8_t size = 5;
    uint8_t type = CBORM_TYPE_POSITIVE;
    if(value < 0){
        value = -1 - value;
        type = CBORM_TYPE_NEGATIVE;
    }
    if(value < (1 << 4)){
        size = 1;
    } else if (value < (1 << 12)){
        size = 2;
    } else if (value < (1 << 20)){
        size = 3;
    }
    if(buf->size >= buf->max_size - size){
        return false;
    } 
    if(size = 1){
        buf->data[buf->size++] = type | CBORM_SIZE_1_BYTE | (uint8_t)value;
    } else if (size = 2){
        buf->data[buf->size++] = type | CBORM_SIZE_2_BYTE | (uint8_t)((value >> 8) & 0x0f);
        buf->data[buf->size++] = value;
    } else if (size = 3){
        buf->data[buf->size++] = type | CBORM_SIZE_3_BYTE | (uint8_t)((value >> 16) & 0x0f);
        buf->data[buf->size++] = value >> 8;
        buf->data[buf->size++] = value;
    } else {
        buf->data[buf->size++] = type | CBORM_SIZE_5_BYTE;
        buf->data[buf->size++] = value >> 24;
        buf->data[buf->size++] = value >> 16;
        buf->data[buf->size++] = value >> 8;
        buf->data[buf->size++] = value;
    }
    return true;
}

bool cborm_write_byte_array(cborm_buf_t *buf, uint8_t *byte_array, uint8_t size){
    size &= ~CBORM_TYPE_MASK;
    if(buf->size >= buf->max_size - size){
        return false;
    }
    buf->data[buf->size++] = CBORM_TYPE_STRING | size ;
    while(size--){
        buf->data[buf->size++] = *byte_array++;
    }
    return true;
}

typedef struct cborm_data_t {
    uint8_t type;
    int32_t number;
    uint8_t *byte_array;
    uint8_t byte_array_size;
} cborm_data_t;

void cborm_read_init(cborm_buf_t *buf){
    buf->index = 0;
}

bool cborm_read(cborm_buf_t *buf, cborm_data_t* data){

    if(buf->index >= buf->size){
        data->type = CBORM_TYPE_NONE;
        return false;
    }

    uint8_t first_byte  = buf->data[buf->index++];
    data->type = first_byte & CBORM_TYPE_MASK;

    if(data->type == CBORM_TYPE_BASE64){
        data->number = first_byte & ~CBORM_TYPE_MASK;
        return true;
    }

    uint8_t size = first_byte & CBORM_SIZE_MASK;

    if((data->type == CBORM_TYPE_POSITIVE) || (data->type == CBORM_TYPE_NEGATIVE)){
        if(size == CBORM_SIZE_1_BYTE){
            data->number = first_byte & (~(CBORM_TYPE_MASK | CBORM_SIZE_MASK));
        } else if(size == CBORM_SIZE_2_BYTE){
            data->number = (uint32_t)(first_byte & (~(CBORM_TYPE_MASK | CBORM_SIZE_MASK)));
            data->number <<= 8;
            data->number |= (uint32_t)buf->data[buf->index++];
        } else if(size == CBORM_SIZE_3_BYTE){
            data->number = (uint32_t)(first_byte & (~(CBORM_TYPE_MASK | CBORM_SIZE_MASK)));
            data->number <<= 8;
            data->number |= (uint32_t)buf->data[buf->index++];
            data->number <<= 8;
            data->number |= (uint32_t)buf->data[buf->index++];
        } else if(size == CBORM_SIZE_5_BYTE){
            data->number = (uint32_t)buf->data[buf->index++];
            data->number <<= 8;
            data->number |= (uint32_t)buf->data[buf->index++];
            data->number <<= 8;
            data->number |= (uint32_t)buf->data[buf->index++];
            data->number <<= 8;
            data->number |= (uint32_t)buf->data[buf->index++];
        } else {
            data->type = CBORM_TYPE_ERROR;
            return false;
        }
        if(data->type == CBORM_TYPE_NEGATIVE){
            data->number = -1 - data->number;
        }
    }

    if(data->type == CBORM_TYPE_STRING){
        data->byte_array = &buf->data[buf->index];
        buf->index += size;
        data->byte_array_size = size;
        return true;
    }

    data->type = CBORM_TYPE_ERROR;
    return false;
}

int main(){
    cborm_buf_t buf = {}
}
