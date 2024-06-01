#include <stdio.h>
#include <string.h>

#include "flash.h"

static FILE* sFD;

static uint8_t readBuffer[flash_kPageSize];
//static uint8_t writeBuffer[flash_kPageSize];
#define writeBuffer readBuffer

static void _seek(uint32_t address) {
    if (fseek(sFD, address, SEEK_SET)) {
        fprintf(stderr, "Error fseek didn't like %d\n", address);
    }
}

uint8_t flash_open(void){
	// create flash file
	char imagePath[16];
	strcpy (imagePath, "flash.dat");

	sFD = fopen(imagePath, "r+");
    if (sFD == NULL) {
        fprintf(stderr, "Couldn't open file '%s', creating it now\n", imagePath);

        // create file
    	sFD = fopen(imagePath, "w+");

    	if (sFD == NULL) {
    		fprintf(stderr, "Flash file open/create failed\n");
       		return flash_kFail;
    	}

    	// populate file
    	char buffer[flash_kPageSize];
    	uint16_t i;
    	for (i = 0 ; i < flash_kPageSize ; i++){
    		buffer[i] = flash_kErasedValue;
    	}
    	for (i = 0 ; i < flash_kPageQuantity ; i++){
    		fwrite(buffer, flash_kPageSize, 1, sFD);
    	}
    }

	return flash_kSuccess;
}

uint8_t flash_close(void){
	fclose(sFD);
	return flash_kSuccess;
}

int16_t flash_read(uint16_t page, uint16_t addr)
{
	_seek(page * flash_kPageSize + addr);
	uint16_t readBuffer;
	uint16_t bytesRead = fread(&readBuffer, 1, 1, sFD);
    if (bytesRead != 1) {
    	return flash_kFail;
    } else {
    	return readBuffer;
    }

}

/*uint8_t flash_readFlashToPageBuffer(uint16_t page)
{
	_seek(page * flash_kPageSize);
	uint16_t bytesRead = fread(readBuffer, 1, flash_kPageSize, sFD);
    if (bytesRead != flash_kPageSize) {
        fprintf(stderr, "Unable to read block\n");
        return flash_kFail;
    }
	return flash_kSuccess;
}
*/

/*uint8_t flash_readPageBuffer(uint16_t addr, uint16_t qty, uint8_t* dest)
{
	if(addr + qty <= flash_kPageSize){
		memcpy(dest, &readBuffer[addr], qty);
		return flash_kSuccess;
	} else {
		return flash_kFail;
	}
}
*/

uint8_t flash_writePageBuffer(uint16_t addr, uint8_t data)
{
	if(addr <= flash_kPageSize){
		writeBuffer[addr] = data;
		return flash_kSuccess;
	} else {
		return flash_kFail;
	}
}

uint8_t flash_writePageBufferToFlash(uint16_t page)
{
	_seek(page * flash_kPageSize);
	uint16_t bytesWritten = fwrite(writeBuffer, 1, flash_kPageSize, sFD);
    if (bytesWritten != flash_kPageSize) {
        fprintf(stderr, "Unable to write block\n");
        return flash_kFail;
    }

	return flash_kSuccess;
}

uint8_t flash_erasePage(uint16_t page){
	uint8_t buffer[flash_kPageSize];

	memset(buffer, flash_kErasedValue, flash_kPageSize);
	_seek(page * flash_kPageSize);
	uint16_t bytesWritten = fwrite(buffer, 1, flash_kPageSize, sFD);
    if (bytesWritten != flash_kPageSize) {
        fprintf(stderr, "Unable to erase block\n");
        return flash_kFail;
    }
	return flash_kSuccess;
}

uint8_t flash_test(void){
	uint8_t bufferOut[flash_kPageSize];
	uint8_t bufferIn[flash_kPageSize];
	uint16_t i, j;

	flash_open();

	for (j = 0 ; j < flash_kPageQuantity ; j++){
		for (i = 0 ; i < flash_kPageSize ; i++){
			bufferOut[i] = (char)(i * j * i);
		}
		//flash_writeBlock(j, bufferOut);
		flash_writePageBuffer(0, flash_kPageSize, bufferOut);
		flash_writePageBufferToFlash(j);
	}

	for (j = 0 ; j < flash_kPageQuantity ; j++){
		for (i = 0 ; i < flash_kPageSize ; i++){
			bufferOut[i] = (char)(i * j * i);
		}
		// flash_readBlock(j, flash_kPageSize, bufferIn);
		flash_readFlashToPageBuffer(j);
		flash_readPageBuffer(0, flash_kPageSize, bufferIn);
		for (i = 0 ; i < flash_kPageSize ; i++){
			if (bufferOut[i] != bufferIn[i]){
				fprintf(stderr, "Fail test!\n");
				return flash_kFail;
			} else {
				// fprintf(stderr, "Match\t%d\t%d\t%d\n", i, j, bufferIn[i] );
			}
		}
	}
	fprintf(stderr, "Flash test successful.\n");
	return flash_kFail;
}
