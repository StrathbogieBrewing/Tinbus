#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "frame.h"

void dump_message(const canbus_message_t *msg) {
    printf("Decoded frame ID : %8.8X  DLC : %d  IDE : %d  RTR : %d  DATA : ", msg->id, msg->dlc, msg->ide, msg->rtr);
    for (int i = 0; i < msg->dlc; i++) {
        printf("%2.2X ", msg->data[i]);
    }
    printf("\n");
}

static const canbus_message_t test_vectors[] = {
    {.id = 0x12345678, .ide = true, .rtr = true, .dlc = 0, .data = {0x00}},
    {.id = 0x1fffffff, .ide = true, .rtr = false, .dlc = 4, .data = {0x12, 0x34, 0x56, 0x78}},
    {.id = 0x0, .ide = true, .rtr = false, .dlc = 8, .data = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0}}};

int main(int argc, char *argv[]) {
    int vectors = sizeof(test_vectors) / sizeof(canbus_message_t);
    for (int i = 0; i < vectors; i++) {
        dump_message(&test_vectors[i]);
        tinbus_frame_t tin_frame;
        assert(frame_enframe(&test_vectors[i], &tin_frame) == FRAME_OK);
        char dump[(TINBUS_BUFFER_SIZE * TINBUS_BITS_IN_BYTE) + 1];
        tinbus_dump(&tin_frame, dump);
        printf("Encoded frame >%s<\n", dump);
        canbus_message_t can_msg_test;
        frame_deframe(&tin_frame, &can_msg_test);
        dump_message(&can_msg_test);
        assert(memcmp(&can_msg_test, &test_vectors[i], sizeof(canbus_message_t)) == 0);
    }
    return EXIT_SUCCESS;
}
