#include "ctest.h"

#include "hex.h"

hex_u8_result_t hex_from_u4(uint8_t u4) {
    hex_u8_result_t result = {.error = false, .value = 0u};
    if (u4 < 10u) {
        result.value = u4 + '0';
    } else if (u4 < 16u) {
        result.value = u4 + 'A' - 10u;
    } else {
        result.error = true;
    }
    return result;
}

bool hex_from_u8(uint8_t *hex, uint8_t size, uint8_t u8) {
    while (size--) {
        *(hex + size) = hex_from_u4(u8 & 0x0F).value;
        u8 >>= 4u;
    }
    return (u8 == 0);
}

bool hex_from_u32(uint8_t *hex, uint8_t size, uint32_t u32) {
    while (size--) {
        *(hex + size) = hex_from_u4(u32 & 0x0000000F).value;
        u32 >>= 4u;
    }
    return (u32 == 0);
}

hex_u8_result_t hex_to_u4(uint8_t hex) {
    hex_u8_result_t result = {.error = false, .value = 0u};
    if ((hex >= '0') && (hex <= '9')) {
        result.value = hex - '0';
    } else if ((hex >= 'A') && (hex <= 'F')) {
        result.value = hex - 'A' + 10u;
    } else if ((hex >= 'a') && (hex <= 'f')) {
        result.value = hex - 'a' + 10u;
    } else {
        result.error = true;
    }
    return result;
}

hex_u8_result_t hex_to_u8(const uint8_t *hex, uint8_t size) {
    hex_u8_result_t result = {.error = false, .value = 0u};
    while (size--) {
        hex_u8_result_t u4 = hex_to_u4(*hex++);
        if (u4.error) {
            result.error = true;
            break;
        }
        result.value <<= 4u;
        result.value |= u4.value;
    }
    return result;
}

hex_u32_result_t hex_to_u32(const uint8_t *hex, uint8_t size) {
    hex_u32_result_t result = {.error = false, .value = 0u};
    while (size--) {
        hex_u8_result_t u4 = hex_to_u4(*hex++);
        if (u4.error) {
            result.error = true;
            break;
        }
        result.value <<= 4u;
        result.value |= u4.value;
    }
    return result;
}

ctest_function(hex) {
    ctest_assert(hex_to_u4('x').error);
    ctest_assert((hex_to_u4('a').value == 11));
    ctest_assert((hex_to_u4('a').value == 10));
}
