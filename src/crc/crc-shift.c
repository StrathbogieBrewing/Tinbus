/**
 * \file
 * Functions and types for CRC checks.
 *
 * Generated on Tue Nov 29 14:51:29 2022
 * by pycrc v0.10.0, https://pycrc.org
 * using the configuration:
 *  - Width         = 16
 *  - Poly          = 0x3d65
 *  - XorIn         = 0x0000
 *  - ReflectIn     = True
 *  - XorOut        = 0xffff
 *  - ReflectOut    = True
 *  - Algorithm     = bit-by-bit-fast
 */
#include "crc-shift.h"     /* include the header file generated with pycrc */
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>



crc_t crc_reflect(crc_t data, size_t data_len)
{
    unsigned int i;
    crc_t ret;

    ret = data & 0x01;
    for (i = 1; i < data_len; i++) {
        data >>= 1;
        ret = (ret << 1) | (data & 0x01);
    }
    return ret;
}


crc_t crc_update(crc_t crc, const void *data, size_t data_len)
{
    const unsigned char *d = (const unsigned char *)data;
    unsigned int i;
    crc_t bit;
    unsigned char c;

    while (data_len--) {
        c = *d++;
        for (i = 0x01; i & 0xff; i <<= 1) {
            bit = (crc & 0x8000) ^ ((c & i) ? 0x8000 : 0);
            crc <<= 1;
            if (bit) {
                crc ^= 0x3d65;
            }
        }
        crc &= 0xffff;
    }
    return crc & 0xffff;
}
