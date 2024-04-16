#ifndef HEX_ASC_H
#define HEX_ASC_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

// returns '0'..'F' for 0..15, '!' otherwise
char hex_bin2asc(uint8_t bin);

// returns 0..15 for '0'..'F', -1 otherwise
int8_t hex_asc2bin(char asc);

#ifdef __cplusplus
}
#endif

#endif /* HEX_ASC_H */