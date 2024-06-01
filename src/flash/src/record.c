#include "record.h"
#include "flash.h"

#define kBadRecordID	(0L)
#define kUsedRecordID	(((kFlash_ErasedValue)L << 0L) + ((kFlash_ErasedValue)L << 8L) + ((kFlash_ErasedValue)L << 16L) + ((kFlash_ErasedValue)L << 24L))

typedef enum {
	kClosed = 0,
	kOpen
} FileState_t;

typedef struct {
	uint32_t recordID;
	uint16_t crc;
	uint16_t offset;
} PageHeader_t;

typedef struct {
	uint8_t offsetFromHeaderRecordID;
	uint8_t length;

} RecordHeader_t;


uint32_t gEarliestRecordID;
uint32_t gLatestRecordID;
uint32_t gCurrentRecordID;
FileState_t gFileOpen = kClosed;

const uint32_t kFileSize = (flash_kBlockSize * flash_kBlockQuantity);

uint8_t record_open(void){

	uint8_t buffer[flash_kBlockSize];
	uint8_t error = 0;
	//PageHeader_t* pageHeader = (PageHeader_t*)buffer;

	uint16_t i;
	for(i = 0 ; i < flash_kBlockQuantity ; i++){
		error = flash_readBlock(i, flash_kBlockSize, buffer);
		if(error != 0){
			break;
		}
	}

	gEarliestRecordID = 0;
	gCurrentRecordID = 0;
	gLatestRecordID = 0;

	if(error == 0){
		gFileOpen = kOpen;
	}
	return error;
}

uint8_t record_format(void){
	return 0;
}

uint8_t record_write(uint8_t* buffer, uint16_t size){
	return 0;
}

uint8_t record_read(uint8_t* buffer, uint16_t maxSize){
	return 0;
}

uint32_t record_getEarliestRecordID(void){
	return gEarliestRecordID;
}

uint32_t record_getLatestRecordID(void){
	return gLatestRecordID;
}

uint32_t record_getCurrentRecordID(void){
	return gCurrentRecordID;
}

uint8_t record_seekRecordID(uint32_t recordID){
	gCurrentRecordID = recordID;
	return 0;
}

uint8_t record_close(void){
	gFileOpen = kClosed;
	return flash_close();
}




uint32_t gSeekIndex;
uint16_t gBlockIndex;
uint16_t gByteIndex;
uint8_t gBlockBuffer[flash_kBlockSize];
uint8_t gBlockStatus;

enum{
	kUpToDate,
	kModified
};


uint8_t file_open(void){
	flash_open();
	gByteIndex = 0;
	gBlockIndex = 0;
	flash_readBlock(gBlockIndex, flash_kBlockSize, gBlockBuffer);
	gBlockStatus = kUpToDate;

	return 0;
}

uint8_t file_write(uint8_t* buffer, uint8_t size){
	uint8_t i;

	for(i = 0 ; i < size ; i++){
		gBlockBuffer[gByteIndex++] = *buffer++;
		gBlockStatus = kModified;
		if(gByteIndex >= flash_kBlockSize){
			flash_writeBlock(gBlockIndex, gBlockBuffer);
			gBlockStatus = kUpToDate;
			gBlockIndex++;
			if(gBlockIndex >= flash_kBlockQuantity){
				gBlockIndex = 0;
			}
			flash_readBlock(gBlockIndex, flash_kBlockSize, gBlockBuffer);
			gByteIndex = 0;
		}
	}

	return 0;
}

uint8_t file_read(uint8_t* buffer, uint8_t size){
	uint8_t i;

	for(i = 0 ; i < size ; i++){
		*buffer++ = gBlockBuffer[gByteIndex++];
		if(gByteIndex >= flash_kBlockSize){
			if(gBlockStatus == kModified){
				flash_writeBlock(gBlockIndex, gBlockBuffer);
				gBlockStatus = kUpToDate;
			}
			if(gBlockIndex >= flash_kBlockQuantity){
				gBlockIndex = 0;
			}
			gBlockIndex++;
			flash_readBlock(gBlockIndex, flash_kBlockSize, gBlockBuffer);
			gByteIndex = 0;
		}
	}

	return 0;
}

uint8_t file_seek(uint32_t index){
	if(gBlockStatus == kModified){
		flash_writeBlock(gBlockIndex, gBlockBuffer);
		gBlockStatus = kUpToDate;
	}
	gBlockIndex = index / flash_kBlockSize;
	gByteIndex = index % flash_kBlockSize;

	flash_readBlock(gBlockIndex, flash_kBlockSize, gBlockBuffer);

	return 0;
}

uint8_t file_close(void){
	if(gBlockStatus == kModified){
		flash_writeBlock(gBlockIndex, gBlockBuffer);
	}
	flash_close();

	return 0;
}

uint8_t file_test(void){
	// uint8_t testImage[kFileSize];
	uint32_t i;

	// populate test image
	for(i = 0 ; i < kFileSize ; i++){
		//testImage[i] = (uint8_t)rand();
	}

	file_open();
	file_seek(0);
	//file_write(testImage, kFileSize);

	for(i = 0 ; i < 100 ; i++){
		// file_seek


	}

	return 0;


}

