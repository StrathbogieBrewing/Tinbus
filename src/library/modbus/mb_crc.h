#ifndef MB_CRC
#define MB_CRC

#include <stdint.h>

typedef union  
{
	uint16_t word_;
	struct {
		uint8_t Lo;
		uint8_t Hi;
	}  bytes_;

} crc_union;



uint16_t mb_crc(uint16_t crc, uint8_t databyte);


/**
	Computes the CRC of the first datalen elements of pData
*/
uint16_t crc16(uint8_t* pData, uint16_t datalen );

/**

	
*/
#ifdef MB_BUILD_ASCII
uint8_t lrc(uint8_t *msg, uint8_t nbytes);
#endif


#endif //MB_CRC_ASDFJKLSDAJFKASDJFIKSADF
