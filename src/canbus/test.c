#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CTEST_ERROR(test, line) do { printf("CTEST Fail in '%s' at line %d\n", test, line); assert(0); } while(0)
#include "ctest.h"

#include "tincan.h"
#include "slcan.h"

static const canbus_message_t test_vectors[] = {
    {.id = 0x12345678, .ide = true, .rtr = true, .dlc = 0, .data = {0x00}},
    {.id = 0x1fffffff, .ide = true, .rtr = false, .dlc = 4, .data = {0x12, 0x34, 0x56, 0x78}},
    {.id = 0x0, .ide = true, .rtr = false, .dlc = 8, .data = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0}}};

int main(int argc, char *argv[]) {

    ctest_run(hex);

    int vectors = sizeof(test_vectors) / sizeof(canbus_message_t);
    for (int i = 0; i < vectors; i++) {
        canbus_dump(&test_vectors[i]);
        tinbus_frame_t tin_frame;
        assert(tincan_enframe(&test_vectors[i], &tin_frame) == TINCAN_OK);
        char dump[(TINBUS_BUFFER_SIZE * TINBUS_BITS_IN_BYTE) + 1];
        tinbus_dump(&tin_frame, dump);
        printf("Frame --> BITS : %s\n", dump);
        canbus_message_t can_msg_test;
        tincan_deframe(&tin_frame, &can_msg_test);
        canbus_dump(&can_msg_test);
        assert(memcmp(&can_msg_test, &test_vectors[i], sizeof(canbus_message_t)) == 0);
        // UNIT_TEST((memcmp(&can_msg_test, &test_vectors[i], sizeof(canbus_message_t)) == 0));


slcan_frame_t slcan;
// slcan_error_t error = 
slcan_enframe(&slcan, &can_msg_test);
slcan.buffer[slcan.size] = '\0';
printf("slcan -->     : %d %s\n", slcan.size, slcan.buffer);

canbus_message_t slcan_msg;
slcan_deframe(&slcan_msg, &slcan);
canbus_dump(&slcan_msg);



        printf("Test passed\n\n");
    }
    return EXIT_SUCCESS;
}
