// #include <assert.h>
// #include <stdio.h>

#include "ctest.h"

#include "tincan.h"

#define TINCAN_HEADER_BYTES (5)

// Order of bits is maintained to be compatible with can bus behaviour
// padding is used to align byte boundaries of data field
// Identifier A                      11 bits   First part of ID (b18 to b28 in extended)
// Identifier Extension Bit (IDE)    1 bit     Recessive (1) for extended frames
// Identifier B                      18        Second part of ID (b0 to b17 in extended)
// Padding for byte alignment        1 bits    Always dominant (0)
// Remote transmission request (RTR) 1 bit     Dominant (0) for normal data frames
// Padding for byte alignment        4 bits    Always dominant (0)
// Data length code (DLC)            4 bits    Number of bytes of data
// Data field                        0–64 bits (0-8 bytes) Data bytes

tincan_error_t tincan_enframe(tinbus_frame_t *tin, const canbus_message_t *can) {
    if (can->ide == false) {
        // currently only support extended frames
        return TINCAN_ERROR;
    }
    if (can->dlc > 8) {
        return TINCAN_ERROR;
    }
    uint32_t id = can->id << 2;
    tin->buffer[3] = id | (can->rtr ? 0b10 : 0b0);
    id >>= 8;
    tin->buffer[2] = id;
    id >>= 8;
    tin->buffer[1] = (id & 0b00001111) | (can->ide ? 0b00010000 : 0);
    id <<= 1;
    tin->buffer[1] |= id & 0b11100000;
    id >>= 8;
    tin->buffer[0] = id;
    uint8_t dlc = can->dlc;
    tin->buffer[4] = dlc;
    tin->length = dlc + TINCAN_HEADER_BYTES;
    while (dlc) {
        dlc--;
        tin->buffer[dlc + TINCAN_HEADER_BYTES] = can->data[dlc];
    }
    return TINCAN_OK;
}

tincan_error_t tincan_deframe(canbus_message_t *can, const tinbus_frame_t *tin) {
    can->ide = tin->buffer[1] & 0b00010000;
    if (can->ide == false) {
        // currently only support extended frames
        return TINCAN_ERROR;
    }
    can->dlc = tin->buffer[4];
    if (can->dlc > 8) {
        return TINCAN_ERROR;
    }
    uint32_t id = tin->buffer[0];
    id <<= 8;
    id |= tin->buffer[1] & 0b11100000;
    id >>= 1;
    id |= tin->buffer[1] & 0b00001111;
    id <<= 8;
    id |= tin->buffer[2];
    id <<= 8;
    id |= tin->buffer[3];
    id >>= 2;
    can->id = id;
    can->rtr = tin->buffer[3] & 0b10;
    uint8_t dlc = can->dlc;
    while (dlc) {
        dlc--;
        can->data[dlc] = tin->buffer[TINCAN_HEADER_BYTES + dlc];
    }
    return TINCAN_OK;
}

CTEST_FUNCTION(tincan_tests) {
    inline bool tincan_test(const canbus_message_t *test_message) {
        tinbus_frame_t tinbus_frame;
        if (tincan_enframe(&tinbus_frame, test_message) != TINCAN_OK) {
            return false;
        }
        canbus_message_t canned_message;
        if (tincan_deframe(&canned_message, &tinbus_frame) != TINCAN_OK) {
            return false;
        }
        uint8_t *test = (uint8_t *)test_message;
        uint8_t *result = (uint8_t *)&canned_message;
        for (int i = 0; i < TINCAN_HEADER_BYTES + test_message->dlc; i++) {
            if (*test++ != *result++) {
                return false;
            }
        }
        return true;
    };

    CTEST_ASSERT(
        tincan_test(&(const canbus_message_t){.id = 0x12345678, .ide = true, .rtr = true, .dlc = 0, .data = {0x00}}));
    CTEST_ASSERT(tincan_test(&(const canbus_message_t){
        .id = 0x1fffffff, .ide = true, .rtr = false, .dlc = 4, .data = {0x12, 0x34, 0x56, 0x78}}));
    CTEST_ASSERT(tincan_test(&(const canbus_message_t){
        .id = 0x0, .ide = true, .rtr = false, .dlc = 8, .data = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0}}));
}