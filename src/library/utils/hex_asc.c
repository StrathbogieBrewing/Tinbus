#include "hex_asc.h"

char hex_bin2asc(uint8_t bin) {
    if (bin < 16) {
        if (bin < 10) {
            return bin + '0';
        } else {
            return bin + 'A' - 10;
        }
    }
    return '!';
}

int8_t hex_asc2bin(char asc) {
    if (asc < '0')
        return -1;
    uint8_t val = asc - '0';
    if (val > 9) {
        val -= 7;
        if ((val > 15) || (val < 10))
            return -1;
    }
    return val;
}