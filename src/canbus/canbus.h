#ifndef CANBUS_H
#define CANBUS_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
  uint32_t id;
  bool ide;
  bool rtr;
  uint8_t dlc;
  uint8_t data[8];
} canbus_message_t;

void canbus_dump(const canbus_message_t *message);

#endif // CANBUS_H