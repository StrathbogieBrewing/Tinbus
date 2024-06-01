#include <stdio.h>

#include "block.h"
#include "flash.h"

uint32_t block_gReadBlock = 0;
uint32_t block_gWriteBlock = 0;

const uint32_t block_kUnusedID = (uint32_t)flash_kErasedValue * 0x01010101UL;
const uint16_t block_kBlockSize = flash_kPageSize;
const uint16_t block_kBlockQuantity = flash_kPageQuantity;
/*
uint8_t block_open(){
	uint16_t i;
	uint32_t blockID = 0;

	// open the flash device
	if(flash_open() != flash_kSuccess){
		return block_kFlashError;
	}

	// find latest record
	for(i = 0 ; i < flash_kPageQuantity ; i++){
		flash_readFlashToPageBuffer(i);
		if(flash_readPageBuffer(0, sizeof(uint32_t), (uint8_t*)(&blockID)) != flash_kSuccess){
			return block_kOpenError;
		}
		if(blockID == block_kUnusedID){
			block_gReadBlock = i;
			block_gWriteBlock = i;
			break;
		}
	}

	return block_kNoError;
}

uint8_t block_close(){
	if(flash_close()){
		return block_kCloseError;
	} else {
		return block_kNoError;
	}
}

uint8_t block_readSeek(void){
	block_gReadBlock = block_gWriteBlock;
	return block_kNoError;
}

uint8_t block_read(uint8_t* buffer){
	uint32_t blockID;

	// move to next oldest record
	if(block_gReadBlock == 0){
		block_gReadBlock = flash_kBlockQuantity - 1;
	} else {
		block_gReadBlock--;
	}
	// read block
	if(flash_readBlock(block_gReadBlock, block_kBlockSize, buffer) != flash_kSuccess){
		return block_kReadError;
	}

	// check that data is valid
	blockID = *((uint32_t*)buffer);
	if(blockID == block_kUnusedID){
		return block_kNoDataError;
	}

	return block_kNoError;
}

uint8_t block_write(uint8_t* buffer){
	uint8_t error = 0;

	// first erase next block
	if(block_gWriteBlock == (flash_kBlockQuantity - 1)){
		error = flash_erasePage(0);
	} else {
		error = flash_erasePage(block_gWriteBlock + 1);
	}

	if(error != flash_kSuccess){
		return block_kEraseError;
	}

	// second write new block

	flash_writePageBufferToFlash(block_gWriteBlock);
	flash_writePageBuffer(uint16_t addr, uint16_t qty, const uint8_t* src);

	if(flash_writeBlock(block_gWriteBlock, buffer) != flash_kSuccess){
		return block_kWriteError;
	}

	// finally update block_gWriteBlock
	if(block_gWriteBlock == flash_kBlockQuantity - 1){
		block_gWriteBlock = 0;
	} else {
		block_gWriteBlock++;
	}

	return block_kNoError;
}

uint8_t block_format(void){
	// ensure that all blocks are erased
	uint16_t i;

	for(i = 0 ; i < flash_kBlockQuantity ; i++){
		if((flash_eraseBlock(i)) != flash_kSuccess){
			return block_kEraseError;
		}
	}

	return block_kNoError;
}

uint8_t block_test(void){
	uint8_t bufferOut[flash_kBlockQuantity][flash_kBlockSize];
	uint8_t bufferIn[flash_kBlockSize];
	uint8_t error = block_kNoError;
	int16_t i, j, k;

	//block_format();

	block_open();

	fprintf(stderr, "Test\tWr = %d\tRd = %d\n", block_gWriteBlock, block_gReadBlock);

	// create some dummy data
	for (j = 0 ; j < flash_kBlockQuantity ; j++){
		for (i = 0 ; i < flash_kBlockSize ; i++){
			bufferOut[j][i] = (char)((i + 1) * (j + 3) * (i + 5));
		}
	}

	for(k = 0 ; k < flash_kBlockQuantity - 1 ; k++){

		for(j = 0 ; j < flash_kBlockQuantity - k ; j++){
			block_write(bufferOut[j]);
		}

		block_readSeek();

		for(j = flash_kBlockQuantity - 1 - k ; j > 0 ; j--){
			block_read(bufferIn);

			for (i = 0 ; i < flash_kBlockSize ; i++){
				if (bufferOut[j][i] != bufferIn[i]){
					fprintf(stderr, "Fail test!\n");
					return -1;
				} else {
					// fprintf(stderr, "Match\t%d\t%d\t%d\n", i, j, bufferIn[i] );
				}
			}
		}
	}

	fprintf(stderr, "Test\tWr = %d\tRd = %d\n", block_gWriteBlock, block_gReadBlock);


/*	for (j = flash_kBlockQuantity - 1 ; j > 10 ; j--){

		for (i = 0 ; i < flash_kBlockSize ; i++){
			bufferOut[i] = (char)(i * j * i);
		}

		block_read(bufferIn);

		for (i = 0 ; i < flash_kBlockSize ; i++){
			if (bufferOut[i] != bufferIn[i]){
				fprintf(stderr, "Fail test!\n");
				return -1;
			} else {
				fprintf(stderr, "Match\t%d\t%d\t%d\n", i, j, bufferIn[i] );
			}
		}
	}
*/
/*
	fprintf(stderr, "Flash test successful.\n");

	block_close();

	return error;
}
*/
