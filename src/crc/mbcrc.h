#ifndef MBCRC_H
#define MBCRC_H

#include <stdint.h>
#include <stdbool.h>

uint16_t mbcrc(uint16_t crc, uint8_t databyte);
bool mbcrc_is_ok(uint8_t data[], uint8_t size);

#endif //MBCRC_H
