/*
 * file.h
 *
 *  Created on: Jul 25, 2010
 *      Author: john
 */

#ifndef RECORD_H_
#define RECORD_H_

#include <stdint.h>



/**
   Initializes the flash record file system and resources associated with it.
   Sets the read record ID to the last record in the flash device.
   Returns zero if successful.
*/
uint8_t record_open(void);

/**
   Format the flash device, erases all data.
   Returns zero if successful.
*/
uint8_t record_format(void);

/**
   Write buffer into a record at the head of the records.
   Returns zero if successful.
*/
uint8_t record_write(uint8_t* buffer, uint16_t size);

/**
   Read current record from flash into buffer and move one record back in the flash device.
   Returns zero if successful.
*/
uint8_t record_read(uint8_t* buffer, uint16_t maxSize);

/**
   Get the recordID of earliest record in flash device.
   Returns recordID of earliest record in flash device.
*/
uint32_t record_getEarliestRecordID(void);

/**
   Get the recordID of latest record in flash device.
   Returns recordID of latest record in flash device.
*/
uint32_t record_getLatestRecordID(void);

/**
   Get the recordID of current record in flash device.
   Returns recordID of current record in flash device.
*/
uint32_t record_getCurrentRecordID(void);

/**
   Goto the recordID in flash device.
   Returns zero if successful.
*/
uint8_t record_seekRecordID(uint32_t recordID);

/**
   Closes the flash record system and releases resources associated with it. Safely saves any data that may be cached.
   Returns zero if successful.
*/
uint8_t record_close(void);


#endif /* FILE_H_ */
