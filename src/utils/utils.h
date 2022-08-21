#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void hexdump(uint8_t buffer[], uint8_t size);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* UTILS_H */
