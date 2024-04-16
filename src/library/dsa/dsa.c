#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <avr/pgmspace.h>

#include "../scpi/scpi.h"
#include "dsa.h"

void dsa_eeprom_write_start(mb_frame_t *mb_frame, uint8_t address, uint32_t value);
bool dsa_eeprom_write_update(mb_frame_t *mb_frame);

#define DSA_EEPROM_SERIAL 0
#define DSA_EEPROM_GAIN 4 // high bytes should be zero. using 32bit eeprom access, but only need 16bit gain
#define DSA_EEPROM_SUBVERSION 8
#define DSA_EEPROM_VERSION 9 // Don't move this byte
#define DSA_EEPROM_DATE 13

// request format is "30<:meas:curr?"
// modbus address range handled is "30<:meas:curr?" to "37<:meas:curr?"
handler_status_t dsa_request(term_str_t request, mb_frame_t *mb_frame) {
    if ((mb_frame->address < HANDLER_MODBUS_DSA_START_ADDRESS) ||
        (mb_frame->address > HANDLER_MODBUS_DSA_END_ADDRESS)) {
        return HANDLER_NOT_FOUND;
    }
    if (strncmp_P(&request[HANDLER_COMMAND_OFFSET], PSTR(SCPI_ID_QUERY), sizeof(SCPI_ID_QUERY)) == 0) {
        strcpy_P(&request[HANDLER_ORIGIN_OFFSET], PSTR(">dsa"));
        return HANDLER_MESSAGE;
    }
    if (strncmp_P(&request[HANDLER_COMMAND_OFFSET], PSTR(":meas:curr?"), sizeof(":meas:curr?")) == 0) {
        mb_frame->function = MB_FUNC_DSA_READ_CURRENT;
        mb_frame->data_size = 0;
        return HANDLER_NO_MESSAGE;
    }
    if (strncmp_P(&request[HANDLER_COMMAND_OFFSET], PSTR(":conf:gain "), sizeof(":conf:gain")) == 0) {
        int32_t gain = atol(&request[HANDLER_COMMAND_OFFSET + sizeof(":conf:gain")]);
        dsa_eeprom_write_start(mb_frame, DSA_EEPROM_GAIN, (uint32_t)gain);
        return HANDLER_NO_MESSAGE;
    }
    if (strncmp_P(&request[HANDLER_COMMAND_OFFSET], PSTR(":conf:gain?"), sizeof(":conf:gain")) == 0) {
        mb_frame->function = MB_FUNC_READ_HOLDING_REG;
        mb_frame->data[0] = 0x00;
        mb_frame->data[1] = DSA_EEPROM_GAIN;
        mb_frame->data[2] = 0x00;
        mb_frame->data[3] = 0x02; // read 2 of the 16 bit registers to get 32 bit value
        mb_frame->data_size = 4;
        return HANDLER_NO_MESSAGE;
    }
    if (strncmp_P(&request[HANDLER_COMMAND_OFFSET], PSTR(":conf:serial "), sizeof(":conf:serial")) == 0) {
        int32_t serial = atol(&request[HANDLER_COMMAND_OFFSET + sizeof(":conf:serial")]);
        dsa_eeprom_write_start(mb_frame, DSA_EEPROM_SERIAL, (uint32_t)serial);
        return HANDLER_NO_MESSAGE;
    }
    if (strncmp_P(&request[HANDLER_COMMAND_OFFSET], PSTR(":conf:serial?"), sizeof(":conf:serial?")) == 0) {
        mb_frame->function = MB_FUNC_READ_HOLDING_REG;
        mb_frame->data[0] = 0x00;
        mb_frame->data[1] = DSA_EEPROM_SERIAL;
        mb_frame->data[2] = 0x00;
        mb_frame->data[3] = 0x02; // read 2 of the 16 bit registers to get 32 bit value
        mb_frame->data_size = 4;
        return HANDLER_NO_MESSAGE;
    }

    strcpy_P(&request[HANDLER_ORIGIN_OFFSET], PSTR(">" SCPI_BAD_QUERY));
    return HANDLER_MESSAGE;
}

// populates response string with modbus response message
handler_status_t dsa_response(term_str_t response, mb_frame_t *mb_frame) {
    if ((mb_frame->address < HANDLER_MODBUS_DSA_START_ADDRESS) ||
        (mb_frame->address > HANDLER_MODBUS_DSA_END_ADDRESS)) {
        return HANDLER_NOT_FOUND;
    }
    response[HANDLER_ADDRESS_HIGH_OFFSET] = (mb_frame->address >> 4) + '0';
    response[HANDLER_ADDRESS_LOW_OFFSET] = (mb_frame->address & 0x0F) + '0';
    response[HANDLER_ORIGIN_OFFSET] = '>';
    response[HANDLER_COMMAND_OFFSET] = '\0';
    if ((mb_frame->function == MB_FUNC_DSA_READ_CURRENT) && (mb_frame->data_size == 3)) {
        int16_t value = (int16_t)(((uint16_t)mb_frame->data[0] << 8) + ((uint16_t)mb_frame->data[1]));
        itoa(value, &response[HANDLER_COMMAND_OFFSET], 10);
        return HANDLER_MESSAGE;
    }
    if (dsa_eeprom_write_update(mb_frame)) {
        return HANDLER_MODBUS_REQUEST;
    } 
    
    if ((mb_frame->function == MB_FUNC_DSA_CALIBRATION) && (mb_frame->data_size == 2)) {
        strcat_P(response, PSTR(SCPI_NO_RESPONSE));
        return HANDLER_MESSAGE;
    }

    if ((mb_frame->function == MB_FUNC_READ_HOLDING_REG) && (mb_frame->data_size == 5) && (mb_frame->data[0] == 4)) {
        uint32_t value = (((uint32_t)mb_frame->data[1] << 24) + ((uint32_t)mb_frame->data[2] << 16) +
                                  ((uint32_t)mb_frame->data[3] << 8) + ((uint32_t)mb_frame->data[4]));
        ultoa(value, &response[HANDLER_COMMAND_OFFSET], 10);
        return HANDLER_MESSAGE;
    }

    return HANDLER_UNSUPPORTED_MODBUS;
}

handler_t dsa_handler = {dsa_request, dsa_response};

// supporting functions for writing 32 bit values to eeprom
#define DSA_EEPROM_ENABLE (0xFF)

static uint8_t dsa_eeprom_data[4] = {0};
static uint8_t dsa_eeprom_address = 0;
static uint8_t dsa_eeprom_bytes_sent = 4;
static bool dsa_eeprom_write_enabled = false;

void dsa_eeprom_write_enable(mb_frame_t *mb_frame) {
    dsa_eeprom_write_enabled = true;
    mb_frame->function = MB_FUNC_DSA_CALIBRATION;
    mb_frame->data[0] = DSA_EEPROM_ENABLE;
    mb_frame->data[1] = DSA_EEPROM_ENABLE;
    mb_frame->data_size = 2;
}

void dsa_eeprom_write_start(mb_frame_t *mb_frame, uint8_t address, uint32_t value) {
    dsa_eeprom_address = address;
    dsa_eeprom_data[3] = value;  // convert to big endian
    value >>= 8;
    dsa_eeprom_data[2] = value;
    value >>= 8;
    dsa_eeprom_data[1] = value;
    value >>= 8;
    dsa_eeprom_data[0] = value;
    dsa_eeprom_bytes_sent = 0;
    dsa_eeprom_write_enable(mb_frame);
}

bool dsa_eeprom_write_update(mb_frame_t *mb_frame) {
    bool status = false;
    if (dsa_eeprom_bytes_sent < sizeof(uint32_t)) {
        if (dsa_eeprom_write_enabled) {
            dsa_eeprom_write_enabled = false;
            mb_frame->function = MB_FUNC_DSA_CALIBRATION;
            mb_frame->data[0] = dsa_eeprom_address++;
            mb_frame->data[1] = dsa_eeprom_data[dsa_eeprom_bytes_sent++];
            mb_frame->data_size = 2;
        } else {
            dsa_eeprom_write_enable(mb_frame);
        }
        status = true;
    }
    return status;
}
