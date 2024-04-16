#ifndef MB_H
#define MB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define MB_ERROR_FLAG (1 << 7)

#define MB_MAX_DATA_SIZE 64

#define MB_FUNC_READ_HOLDING_REG (0x03)
#define MB_FUNC_WRITE_SINGLE_REG (0x06)
#define MB_FUNC_READ_WRITE_MULTIPLE_REG (0x17)

// DSA function codes
#define MB_FUNC_DSA_READ_CURRENT (0x64)
#define MB_FUNC_DSA_DISCOVERY 112
#define MB_FUNC_DSA_WRITE_MB_ADDR 113
#define MB_FUNC_DSA_READ_SERIAL_NO 114
#define MB_FUNC_DSA_CALIBRATION 120

// SCPI function codes
#define MB_FUNC_SCPI_WRITE_READ (0x65)

typedef struct mb_frame_t {
    uint8_t address;
    uint8_t function;
    uint8_t data_size;
    uint8_t data[MB_MAX_DATA_SIZE];
} mb_frame_t;

typedef enum {
    MB_BUSY = 0,
    MB_FRAME_GOOD,
    MB_FRAME_BAD,
} mb_status;

#ifdef __cplusplus
}
#endif

#endif /* MB_H */
