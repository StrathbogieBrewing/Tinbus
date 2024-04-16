#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include <avr/pgmspace.h>

#include "relay.h"
#include "scpi.h"

#define RELAY_ON (0x0100)
#define RELAY_OFF (0x0200)

#define RELAY_COMMAND_STRING ":relay"
#define RELAY_COMMAND_STRING_SIZE (sizeof(RELAY_COMMAND_STRING) - 1)

#define RELAY_INDEX_OFFSET (HANDLER_COMMAND_OFFSET + RELAY_COMMAND_STRING_SIZE)

#define RELAY_STATE_ON_STRING "1"
#define RELAY_STATE_OFF_STRING "0"
#define RELAY_STATE_OFFSET (RELAY_INDEX_OFFSET + 2)
#define RELAY_QUERY_OFFSET (RELAY_INDEX_OFFSET + 1)

static const char relay_state_on_string[] = RELAY_STATE_ON_STRING;
static const char relay_state_off_string[] = RELAY_STATE_OFF_STRING;

// request format is "01<:relay1 1" or "01<:relay1 0" etc...
// modbus address range handled is "01<:relay1 1" to "09<:relay8 0"
handler_status_t relay_request(term_str_t request, mb_frame_t *mb_frame) {
    if ((mb_frame->address < HANDLER_MODBUS_RELAY_START_ADDRESS) ||
        (mb_frame->address > HANDLER_MODBUS_RELAY_END_ADDRESS)) {
        return HANDLER_NOT_FOUND;
    }
    if (strncmp_P(&request[HANDLER_COMMAND_OFFSET], PSTR("*idn?"), sizeof("*idn?")) == 0) {
        strcpy_P(&request[HANDLER_ORIGIN_OFFSET], PSTR(">relay"));
        return HANDLER_MESSAGE;
    }
    if (strncmp_P(&request[HANDLER_COMMAND_OFFSET], PSTR(RELAY_COMMAND_STRING), RELAY_COMMAND_STRING_SIZE) != 0) {
        strcpy_P(&request[HANDLER_ORIGIN_OFFSET], PSTR(">!not-supported"));
        return HANDLER_MESSAGE;
    }
    char relay_index_char = request[RELAY_INDEX_OFFSET];
    if (!isdigit(relay_index_char)) {
        strcpy_P(&request[HANDLER_ORIGIN_OFFSET], PSTR(">!index-not-numeric"));
        return HANDLER_MESSAGE;
    }
    uint8_t relay_index = (uint8_t)relay_index_char - '0';
    if ((relay_index < 1) || (relay_index > 8)) {
        relay_index = 0;
        strcpy_P(&request[HANDLER_ORIGIN_OFFSET], PSTR(">!index-out-of-range"));
        return HANDLER_MESSAGE;
    }
    if (request[RELAY_QUERY_OFFSET] == '?') {
        mb_frame->function = MB_FUNC_READ_HOLDING_REG;
        mb_frame->data[0] = 0x00;
        mb_frame->data[1] = relay_index;
        mb_frame->data[2] = 0x00;
        mb_frame->data[3] = 0x01;
        mb_frame->data_size = 4;
    } else {
        mb_frame->function = MB_FUNC_WRITE_SINGLE_REG;
        mb_frame->data[0] = 0x00;
        mb_frame->data[1] = relay_index;
        if (strcmp(&request[RELAY_STATE_OFFSET], relay_state_on_string) == 0) {
            mb_frame->data[2] = (uint8_t)(RELAY_ON >> 8);
            mb_frame->data[3] = (uint8_t)RELAY_ON;
        } else if (strcmp(&request[RELAY_STATE_OFFSET], relay_state_off_string) == 0) {
            mb_frame->data[2] = (uint8_t)(RELAY_OFF >> 8);
            mb_frame->data[3] = (uint8_t)RELAY_OFF;
        } else {
            strcpy_P(&request[HANDLER_ORIGIN_OFFSET], PSTR(">!bad-value"));
            return HANDLER_MESSAGE;
        }
        mb_frame->data_size = 4;
    }
    return HANDLER_NO_MESSAGE;
}

// populates response string with modbus message
handler_status_t relay_response(term_str_t response, mb_frame_t *mb_frame) {
    if ((mb_frame->address < HANDLER_MODBUS_RELAY_START_ADDRESS) ||
        (mb_frame->address > HANDLER_MODBUS_RELAY_END_ADDRESS)) {
        return HANDLER_NOT_FOUND;
    }
    response[HANDLER_ADDRESS_HIGH_OFFSET] = (mb_frame->address >> 4) + '0';
    response[HANDLER_ADDRESS_LOW_OFFSET] = (mb_frame->address & 0x0F) + '0';
    response[HANDLER_ORIGIN_OFFSET] = '>';
    response[HANDLER_COMMAND_OFFSET] = '\0';
    if ((mb_frame->function == MB_FUNC_WRITE_SINGLE_REG) && (mb_frame->data_size == 4)){
        strcat_P(response, PSTR(SCPI_NO_RESPONSE));
        return HANDLER_MESSAGE;
    }
    if ((mb_frame->function == MB_FUNC_READ_HOLDING_REG) && (mb_frame->data_size == 3)) {
        uint16_t relay_state = ((uint16_t)mb_frame->data[2] << 8) + mb_frame->data[3];
        if (relay_state & RELAY_ON) {
            strcat(response, relay_state_on_string);
            return HANDLER_MESSAGE;
        } else {
            strcat(response, relay_state_off_string);
            return HANDLER_MESSAGE;
        }
    }
    return HANDLER_UNSUPPORTED_MODBUS;
}

handler_t relay_handler = {relay_request, relay_response};