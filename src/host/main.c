#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>

#include "../cobsm/cobsm.h"
#include "../crc/mbcrc.h"

#define BUFFER_SIZE 1024

static volatile int keepRunning = 1;

void intHandler(int dummy) {
    keepRunning = 0;
}

uint32_t get_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    tv.tv_sec -= (tv.tv_sec / (uint64_t)(24 * 60 * 60)) * (uint64_t)(24 * 60 * 60); // remove days
    return tv.tv_sec * (uint64_t)1000 + tv.tv_usec / (uint64_t)1000;
}

int main(int argc, char **argv) {
    uint8_t buffer[BUFFER_SIZE];
    uint16_t buffer_index = 0;
    fprintf(stdout, "time_ms,current_ma,voltage_mv\n");
    while (keepRunning) {
        int ch = 0;
        ch = getchar();
        if (ch == EOF) {
            break;
        } else if (ch == '\0') { // process frame
            uint8_t size = cobsm_decode(buffer, buffer_index);
            if (mbcrc_is_ok(buffer, size)) {
                uint8_t id = buffer[0];
                int16_t value = (buffer[1] << 8) | buffer[2];
                switch (id) {
                case 0x01: // current
                    fprintf(stdout, "%d,%d,\n", get_time_ms(), (int)((float)value * 24.9));
                    break;
                case 0x02: // voltage
                    fprintf(stdout, "%d,,%d\n", get_time_ms(), (int)((float)value * 2.482));
                    break;
                }
            }
            buffer_index = 0; // reset buffer for next frame
        } else {
            buffer[buffer_index] = (uint8_t)ch; // add charachter to buffer
            buffer_index++;
            if (buffer_index == BUFFER_SIZE) {
                buffer_index = 0; // wrap on buffer overflow
            }
        }
    }
    return EXIT_SUCCESS;
}