/*
 ============================================================================
 Name        : log.c
 Author      : 
 Version     :
 Description : Data logging file system
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

#include "log.h"
#include "flash.h"

//static uint16_t gCurrentPage;
//static uint8_t gBuffer[flash_kBlockSize];

typedef struct{
	uint32_t time;
	uint16_t dataOffset;
	uint16_t crc;

} PageHeader;

void log_putRecord(Packet* packet){

	uint8_t headerBuffer[kMaxBlobHeaderSize];
//	uint8_t* blobBuffer = blob->buffer;
	blob->buffer = headerBuffer;

	//uint32_t timeNow = rtc_getTime();
	//blob->time = timeNow - page_lastEntryTime();
//	uint8_t headerSize = blob_encodeHeader(blob);

	//page_putData(headerBuffer, headerSize);
//	page_putData(blobBuffer, size);
}

uint8_t log_getRecord(Packet* blob){
	return 0;
}

void log_seekRecord(void){

}


