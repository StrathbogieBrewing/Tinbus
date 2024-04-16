/*

 */

#include <stdint.h>

#include "mb_crc.h"

#ifdef MB_CRC_AVR

uint16_t mb_crc(uint16_t __crc, uint8_t __data)
{
	uint8_t __tmp;
	uint16_t __ret;

	__asm__ __volatile__(
		"eor %A0,%2"
		"\n\t"
		"mov %1,%A0"
		"\n\t"
		"swap %1"
		"\n\t"
		"eor %1,%A0"
		"\n\t"
		"mov __tmp_reg__,%1"
		"\n\t"
		"lsr %1"
		"\n\t"
		"lsr %1"
		"\n\t"
		"eor %1,__tmp_reg__"
		"\n\t"
		"mov __tmp_reg__,%1"
		"\n\t"
		"lsr %1"
		"\n\t"
		"eor %1,__tmp_reg__"
		"\n\t"
		"andi %1,0x07"
		"\n\t"
		"mov __tmp_reg__,%A0"
		"\n\t"
		"mov %A0,%B0"
		"\n\t"
		"lsr %1"
		"\n\t"
		"ror __tmp_reg__"
		"\n\t"
		"ror %1"
		"\n\t"
		"mov %B0,__tmp_reg__"
		"\n\t"
		"eor %A0,%1"
		"\n\t"
		"lsr __tmp_reg__"
		"\n\t"
		"ror %1"
		"\n\t"
		"eor %B0,__tmp_reg__"
		"\n\t"
		"eor %A0,%1"
		: "=r"(__ret), "=d"(__tmp)
		: "r"(__data), "0"(__crc)
		: "r0");
	return __ret;
}

#else

/** \ingroup util_crc
	Optimized CRC-16 calculation.

	Polynomial: x^16 + x^15 + x^2 + 1 (0xa001)<br>
	Initial value: 0xffff

	This CRC is normally used in disk-drive controllers.

	The following is the equivalent functionality written in C.
	If the natural byte order is correct for mb, you can leave
	out the byte swapping code.
*/
uint16_t mb_crc(uint16_t crc, uint8_t databyte)
{
	int i;
	// crc_union crc_;

	crc = crc ^ databyte;
	for (i = 0; i < 8; ++i)
	{
		if (crc & 1)
			crc = (crc >> 1) ^ 0xA001;
		else
			crc = (crc >> 1);
	}

	return crc;

	// this does not optimise out.
	// crc_.bytes_.Hi = crc >> 8;
	// crc_.bytes_.Lo = crc & 0xFF;
	// return (crc_.word_);
}

#endif



