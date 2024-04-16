#ifndef FORMAT_H
#define FORMAT_H

#include <stdint.h>

#define FORMAT_STR_MAX_SIZE 8

typedef enum format_t {
    FORMAT_UNIT = 0,
    FORMAT_DECI,
    FORMAT_CENTI,
    FORMAT_MILLI,
    FORMAT_BOOL,
    FORMAT_TWO_DIGITS,
    FORMAT_TIME,
} format_t;

typedef char format_str_t[FORMAT_STR_MAX_SIZE];

uint8_t format_toString(format_str_t str, int16_t value, format_t format);

#endif