#ifndef HEX_H
#define HEX_H

#include <stdint.h>
#include <stdbool.h>

#define HEX_MAX_CHARACTERS (8)

typedef struct hex_u8_result_t {
    bool error;
    uint8_t value;
} hex_u8_result_t;

typedef struct hex_u32_result_t {
    bool error;
    uint32_t value;
} hex_u32_result_t;

hex_u8_result_t hex_from_u4(uint8_t u4);
hex_u8_result_t hex_to_u4(uint8_t hex);

bool hex_from_u8(uint8_t *hex, uint8_t size, uint8_t u8);
hex_u8_result_t hex_to_u8(const uint8_t *hex, uint8_t size);

bool hex_from_u32(uint8_t *hex, uint8_t size, uint32_t u32);
hex_u32_result_t hex_to_u32(const uint8_t *hex, uint8_t size);

#endif // HEX_H