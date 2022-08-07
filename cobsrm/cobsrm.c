#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define COBSRM_BUFFER_SIZE (32)

// typedef struct buffer_t {
//     uint8_t data[COBSRM_BUFFER_SIZE];
//     uint8_t size;
// } buffer_t;

static uint8_t bin2hex(uint8_t bin) {
  bin &= 0x0F;
  if (bin < 10) {
    return bin + '0';
  } else {
    return bin + 'A' - 10;
  }
}

static void hexdump(uint8_t buffer[], uint8_t size) {
  uint8_t index = 0;
  putchar(bin2hex(size >> 4));
  putchar(bin2hex(size));
  putchar('\t');
  while (index < size) {
    putchar(bin2hex(buffer[index] >> 4));
    putchar(bin2hex(buffer[index++]));
    putchar(' ');
  }
  putchar('\n');
}

/* The buffer must be one byte bigger than the payload size */
uint8_t cobsrm_encode(uint8_t buffer[], uint8_t size) {
  uint8_t code = 0;
  uint8_t index = 0;
  uint8_t code_index = 0;
  buffer[size] = buffer[index++];
  while (index <= size) {
    if (buffer[index] == 0) {
      buffer[code_index] = code + 1;
      code_index = index;
      code = 0;
    } else {
      code++;
    }
    index++;
  }
  if (buffer[index - 1] > code) {
    buffer[code_index] = buffer[--index];
  } else {
    buffer[code_index] = code + 1;
  }
  return index;
}

uint8_t cobsrm_decode(uint8_t *buffer, uint8_t size) {
  uint8_t index = 0;
  uint8_t code_index = buffer[0];
  uint8_t code = buffer[code_index];
  if (code_index > size) {
    return size;
  }
  while (index < size) {
    if (index == code_index) {
      code = buffer[index];
      buffer[index] = 0;
      code_index = index + code;
    }
    index++;
  }
  if (code_index <= size) {
    size--;
  } 
  buffer[0] = buffer[size];
  return size;
}

struct tests {
  const uint8_t *test_vector;
  const uint8_t test_size;
};

static const uint8_t test_0[] = {0, 0, 0, 0};
static const uint8_t test_1[] = {'1', '2', '3', '4', '5'};
static const uint8_t test_2[] = {'1', '2', '3', '4', '5', 0};
static const uint8_t test_3[] = {'1', '2', '3', '4', '5', 0, '7', '8', '9'};
static const uint8_t test_4[] = {'1', '2', '3', '4', '5', 0,
                                 '6', '7', '8', '9', 0};
static const uint8_t test_5[] = {2, '3', '4', '5', 0, '6', '7', '8', '9', 0};
static const uint8_t test_6[] = {1, 2, 3, 4};
static const uint8_t test_7[] = {4, 2, 3, 4, 0};
static const uint8_t test_8[] = {7, 0, 3, 0, 0, 5};
static const uint8_t test_9[] = {3, 2, 0, 3, 4, 0, 5, 6, 7};

struct tests test_list[] = {
    {test_0, sizeof(test_0)}, {test_1, sizeof(test_1)},
    {test_2, sizeof(test_2)}, {test_3, sizeof(test_3)},
    {test_4, sizeof(test_4)}, {test_5, sizeof(test_5)},
    {test_6, sizeof(test_6)}, {test_7, sizeof(test_7)},
    {test_8, sizeof(test_8)}, {test_9, sizeof(test_9)},
};

uint8_t test_count = sizeof(test_list) / sizeof(struct tests);

int main() {
  // const uint8_t *test = test_0;
  uint8_t buffer[256] = {0};

  uint8_t test_index = 0;
  while (test_index < test_count) {
    memcpy(buffer, test_list[test_index].test_vector,
           test_list[test_index].test_size);
    hexdump(buffer, test_list[test_index].test_size);
    uint8_t size = cobsrm_encode(buffer, test_list[test_index].test_size);
    hexdump(buffer, size);
    size = cobsrm_decode(buffer, size);
    hexdump(buffer, size);
    if (test_list[test_index].test_size != size) {
      puts("FAIL");
    } else {
      if (memcmp(test_list[test_index].test_vector, buffer,
                 test_list[test_index].test_size)) {
        puts("FAIL");
      }
    }

    putchar('\n');
    test_index++;
  }

  return 0;
}