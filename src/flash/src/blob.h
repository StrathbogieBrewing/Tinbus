#ifndef BLOB_H_
#define BLOB_H_

#include <stdint.h>

typedef struct {
	uint32_t time;
	uint8_t size;
} blob_Meta_t;

#define blob_kMaxBlobSize		16
#define blob_kMaxBlobPayload	(blob_kMaxBlobSize - sizeof(blob_Meta_t))

typedef struct {
	blob_Meta_t meta;
	uint8_t buffer[blob_kMaxBlobPayload];
} blob_Blob_t;

uint8_t writeBuffer[];
uint8_t readBuffer[];

uint8_t blob_write(uint8_t size, uint8_t* buffer);
uint8_t blob_read(*buffer);
uint8_t blob_seek(void);
uint8_t blob_open(void);
uint8_t blob_close(void);
uint8_t blob_erase(void);
uint8_t blob_test(void);

uint8_t blob_encodeHeader(Blob* header);
uint8_t blob_decodeHeader(Blob* header);

void blob_test(void);

#endif /* BLOB_H_ */
