/*
 * log.h
 *
 *  Created on: Jul 20, 2010
 *      Author: john
 */

#ifndef LOG_H_
#define LOG_H_

#include <stdint.h>

typedef struct {
	uint8_t size;
	uint8_t buffer[];
} Packet;

uint8_t log_putRecord(Packet* blob);
uint8_t log_getRecord(Packet* blob);
uint8_t log_seekRecord(void);


#endif /* LOG_H_ */
