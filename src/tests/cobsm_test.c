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
    uint8_t size = cobsm_encode(buffer, test_list[test_index].test_size);
    hexdump(buffer, size);
    size = cobsm_decode(buffer, size);
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