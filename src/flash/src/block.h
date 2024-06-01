/*
 * block.h
 *
 *  Created on: Jul 25, 2010
 *      Author: john
 */

#ifndef BLOCK_H_
#define BLOCK_H_

#include<stdint.h>

extern const uint16_t block_kBlockSize;

enum {
	block_kNoError = 0,
	block_kWriteError,
	block_kEraseError,
	block_kReadError,
	block_kNoDataError,
	block_kOpenError,
	block_kFlashError,
	block_kCloseError,
};

/**
   Initializes the flash memory system and resources associated with it.
   Returns zero if successful.
*/
uint8_t block_open(void);

/**
   Closes the flash memory system and releases resources associated with it.
   Returns zero if successful.
*/
uint8_t block_close(void);

/**
   Move to the current read block index to the most recently written block.
   Returns zero if successful.
*/
uint8_t block_readSeek(void);

/**
   Read the current read block and move the read index to the next oldest block.
   Returns zero if successful.
*/
uint8_t block_read(uint8_t* buffer);

/**
   Write a new block to the flash file system.
   Returns zero if successful.
*/
uint8_t block_write(uint8_t* buffer);

/**
   Format the flash file system.
   Returns zero if successful.
*/
uint8_t block_format(void);

/**
   Coverage test for the flash file system.
   Returns zero if successful.
*/
uint8_t block_test(void);



#endif
