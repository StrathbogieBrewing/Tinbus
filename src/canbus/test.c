#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ctest.h"

#include "slcan.h"
#include "tincan.h"

void ctest_result(const char *name, int line) {
    if (line < 0) {
        printf("CTEST Fail in %s line %d\n", name, -line);
    }
}

int main(int argc, char *argv[]) {
    CTEST_RUN(hex_tests);
    CTEST_RUN(tincan_tests);
    CTEST_RUN(hex_tests_fail);

    // const canbus_message_t can_msg_test = {.id = 0x12345678, .ide = true, .rtr = true, .dlc = 0, .data = {0x00}};
    const canbus_message_t can_msg_test = {
        .id = 0x1fffffff, .ide = true, .rtr = false, .dlc = 4, .data = {0x12, 0x34, 0x56, 0x78}};
    tinbus_frame_t tin;
    tincan_enframe(&tin, &can_msg_test);
    tinbus_dump_t dump;
    tinbus_dump(dump, &tin);
    printf("tinbus -->     :   %d   %s\n", tin.length, dump);

    slcan_frame_t slcan;
    // slcan_error_t error =
    slcan_enframe(&slcan, &can_msg_test);
    slcan.buffer[slcan.size] = '\0';
    printf("slcan -->     : %d %s\n", slcan.size, slcan.buffer);

    canbus_message_t slcan_msg;
    slcan_deframe(&slcan_msg, &slcan);
    canbus_dump(&slcan_msg);

    return EXIT_SUCCESS;
}
