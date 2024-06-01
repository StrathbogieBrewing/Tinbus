/*
 * flash.h
 *
 *  Created on: Jul 20, 2010
 *      Author: john
 */

#ifndef FLASH_H_
#define FLASH_H_

#include <stdint.h>

#define flash_kPageSize			641
#define flash_kPageQuantity		1289
#define flash_kErasedValue		0xFF

#define flash_kSuccess			0
#define flash_kFail				(-1)

/**
   Initializes the flash memory system and resources associated with it.
   Returns zero if successful.
*/
uint8_t flash_open(void);

/**
   Reads byte of data from page, addr.
   Returns value in flash if successful or flash_kFail if unsuccessful.
*/
int16_t flash_read(uint16_t page, uint16_t addr);

/**
   Erases the data in the designated flash block.
   Returns zero if successful.
*/
uint8_t flash_erasePage(uint16_t page);

/**
   Writes data in write page buffer to the designated flash page.
   Returns zero if successful. This function should verify that the write was successful.
*/
uint8_t flash_writePageBufferToFlash(uint16_t page);


/**
   Writes qty bytes of data from src to the write page buffer location given by addr.
   Returns zero if successful.
*/
uint8_t flash_writePageBuffer(uint16_t addr, uint8_t data);

/**
   Closes the flash memory system and releases resources associated with it.
   Returns zero if successful.
*/
uint8_t flash_close(void);



uint8_t flash_test(void);


#endif /* FLASH_H_ */
