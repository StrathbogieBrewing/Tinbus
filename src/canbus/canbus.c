#include <stdio.h>
#include "canbus.h"

void canbus_dump(const canbus_message_t *msg) {
    printf("Frame --> ID   : %8.8X  DLC : %d  IDE : %d  RTR : %d  DATA : ", msg->id, msg->dlc, msg->ide, msg->rtr);
    for (int i = 0; i < msg->dlc; i++) {
        printf("%2.2X ", msg->data[i]);
    }
    printf("\n");
}