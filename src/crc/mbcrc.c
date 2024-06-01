#include <stdint.h>

#include "mbcrc.h"

uint16_t mbcrc(uint16_t crc, uint8_t databyte) {
    int i;
    crc = crc ^ databyte;
    for (i = 0; i < 8; ++i) {
        if (crc & 1)
            crc = (crc >> 1) ^ 0xA001;
        else
            crc = (crc >> 1);
    }
    return crc;
}

bool mbcrc_is_ok(uint8_t data[], uint8_t size){
	uint16_t crc = 0xFFFF;
	for(uint8_t i = 0; i < size; i++){
		crc = mbcrc(crc, data[i]);
	}
	if(crc){
		return false;
	}
	return true;
}