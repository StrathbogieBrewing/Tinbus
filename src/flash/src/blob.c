/*
 * packer.c
 *
 *  Created on: Jul 22, 2010
 *      Author: john
 */

#include <stdio.h>

#include "blob.h"

enum {
	kNoPreset = -1,
	kZeroSeconds = 0,
	kOneSecond = 1,
	kFiveSeconds = 5,
	kTenSeconds = 10,
	kFifteenSeconds = 15,
	kThirtySeconds = 30,
	kOneMinute = 60,
	kFiveMinutes = 5 * 60,
	kTenMinutes = 10 * 60,
	kFifteenMinutes = 15 * 60,
	kThirtyMinutes = 30 * 60,
	kOneHour = 60 * 60,
	kSixHours = 6 * 60 * 60,
};

const int16_t kDeltaTimes[] = {
		kNoPreset,
		kZeroSeconds,
		kOneSecond,
		kFiveSeconds,
		kTenSeconds,
		kFifteenSeconds,
		kThirtySeconds,
		kOneMinute,
		kFiveMinutes,
		kTenMinutes,
		kFifteenMinutes,
		kThirtyMinutes,
		kOneHour,
		kSixHours };


// returns the size of the constructed header in bytes
uint8_t blob_encodeHeader(Blob* header){
	uint8_t i;
	uint8_t* pHeader = header->buffer + 1;

	uint8_t sizeMeta = kExtend8Bits;
	for (i = 0 ; i < kPresets ; i++){
		if(header->size == kBlobSizes[i]){
			sizeMeta = i;
			break;
		}
	}
	if(sizeMeta == kExtend8Bits){
		*pHeader = header->size;
		pHeader += sizeof(uint8_t);
	}

	uint8_t timeMeta = kExtend32Bits;
	for (i = 0 ; i < kPresets ; i++){
		if(header->time == kBlobTimes[i]){
			timeMeta = i;
			break;
		}
	}
	if(timeMeta == kExtend32Bits){
		if(header->time <= UINT8_MAX){
			*pHeader = (uint8_t)(header->time);
			timeMeta = kExtend8Bits;
			pHeader += sizeof(uint8_t);
		} else if (header->time <= UINT16_MAX){
			*(uint16_t*)pHeader = (uint16_t)header->time;
			timeMeta = kExtend16Bits;
			pHeader += sizeof(uint16_t);
		} else {
			*(uint32_t*)pHeader = header->time;
			timeMeta = kExtend32Bits;
			pHeader += sizeof(uint32_t);
		}
	}

	*(header->buffer) = sizeMeta | (timeMeta << 4);
	return pHeader - header->buffer;
}

uint8_t blob_decodeHeader(Blob* header){
	uint8_t* pHeader = header->buffer;
	uint8_t sizeMeta = *pHeader & 0x0F;
	uint8_t timeMeta = ((*pHeader) >> 4) & 0x0F;
	pHeader++;

	if(sizeMeta < kPresets){
		header->size = kBlobSizes[sizeMeta];
	} else {
		header->size = *pHeader;
		pHeader += sizeof(uint8_t);
	}

	if(timeMeta < kPresets){
		header->time = (uint32_t)kBlobTimes[timeMeta];
	} else if (timeMeta == kExtend8Bits){
		header->time = (uint32_t)(*pHeader);
		pHeader += sizeof(uint8_t);
	}  else if (timeMeta == kExtend16Bits){
		header->time = (uint32_t)*((uint16_t*)pHeader);
		pHeader += sizeof(uint16_t);
	} else if (timeMeta == kExtend32Bits){
		header->time = *((uint32_t*)pHeader);
		pHeader += sizeof(uint32_t);
	}

	return pHeader - header->buffer;
}

void blob_dump(Blob* header){
	uint8_t i;
	uint8_t* pHeader = header->buffer;

	printf("Size : %d\tTime : %d\tData : ", header->time, header->size);

	for(i = 0 ; i < kMaxBlobHeaderSize ; i++){
		printf("%2.2x ", *pHeader++);
	}
	printf("\n");
}

void blob_test(void){
	uint8_t buffer[kMaxBlobHeaderSize];

	Blob header;
	uint8_t headerSize;

	uint8_t size = 1;
	uint32_t time = 1;

	for(size = 1 ; size < 18 ; size++){
		for(time = 0 ; time < 300 ; time += 10){
			header.buffer = buffer;
			header.size = size;
			header.time = time;

			headerSize = blob_encodeHeader(&header);
			printf("Header size : %d\t", headerSize);
			blob_dump(&header);

			header.buffer = buffer;
			header.size = 0;
			header.time = 0;

			if(blob_decodeHeader(&header) != headerSize){
				printf("Failed - header  %d, %d\n", blob_decodeHeader(&header), headerSize);
				return;
			}
			if(header.size != size){
				printf("Failed - size = %d, %d\n", header.size, size);
				return;
			}
			if(header.time != time){
				printf("Failed - time %d, %d\n", header.time, time);
				return;
			}
		}

	}
	printf("Completed test successfully.\n");
}
