#ifndef COBSM_H
#define COBSM_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint8_t cobsm_encode(uint8_t buffer[], uint8_t size);
uint8_t cobsm_decode(uint8_t *buffer, uint8_t size);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* COBSM_H */
