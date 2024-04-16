#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <avr/pgmspace.h>

#include "scpi.h"
#include "term.h"
#include "timer.h"

static uint32_t scpi_tx_time_ticks = 0;

handler_status_t scpi_request(term_str_t request, mb_frame_t *mb_frame) {
    if ((mb_frame->address < HANDLER_MODBUS_SCPI_START_ADDRESS) ||
        (mb_frame->address > HANDLER_MODBUS_SCPI_END_ADDRESS)) {
        return HANDLER_NOT_FOUND;
    }
    uint8_t request_size = strlen(request) - HANDLER_COMMAND_OFFSET;
    mb_frame->function = MB_FUNC_SCPI_WRITE_READ;
    mb_frame->data[0] = request_size; // bytes in request to write to slave
    memcpy(&mb_frame->data[1], &request[HANDLER_COMMAND_OFFSET], request_size);
    mb_frame->data_size = request_size + 1; // include the number of bytes
    return HANDLER_NO_MESSAGE;
}

handler_status_t scpi_response(term_str_t response, mb_frame_t *mb_frame) {
    if ((mb_frame->address < HANDLER_MODBUS_SCPI_START_ADDRESS) ||
        (mb_frame->address > HANDLER_MODBUS_SCPI_END_ADDRESS)) {
        return HANDLER_NOT_FOUND;
    }
    response[HANDLER_ADDRESS_HIGH_OFFSET] = (mb_frame->address >> 4) + '0';
    response[HANDLER_ADDRESS_LOW_OFFSET] = (mb_frame->address & 0x0F) + '0';
    if (mb_frame->function != MB_FUNC_SCPI_WRITE_READ) {
        return HANDLER_UNSUPPORTED_MODBUS;
    }
    uint8_t response_size = mb_frame->data[0];
    if ((response_size + 1) != mb_frame->data_size) {
        strcpy_P(&response[HANDLER_ORIGIN_OFFSET], PSTR("^!modbus-response-wrong-size"));
        return HANDLER_MESSAGE;
    }
    if (response_size >= (TERM_STR_MAX_SIZE - 4)) { // need space for address, '>' and terminating null
        strcpy_P(&response[HANDLER_ORIGIN_OFFSET], PSTR("^!modbus-response-too-big"));
        return HANDLER_MESSAGE;
    }
    response[HANDLER_ORIGIN_OFFSET] = '>';
    memcpy(&response[HANDLER_COMMAND_OFFSET], &mb_frame->data[1], response_size);
    response[HANDLER_COMMAND_OFFSET + response_size] = '\0';
    return HANDLER_MESSAGE;
}

void scpi_tx(FILE *fd, term_str_t str) {
    fputs(str, fd);
    fputc('\n', fd);
    scpi_tx_time_ticks = timer_get_ticks();
    if (scpi_tx_time_ticks == 0) {
        scpi_tx_time_ticks = 1; // reserve '0' for disabled timeout
    }
}

scpi_status scpi_rx(FILE *fd, scpi_ctx_t *ctx) {
    scpi_status status = SCPI_BUSY;
    int rx_byte = fgetc(fd);
    if (rx_byte != EOF) {
        if (rx_byte == '\n') {
            ctx->data[ctx->data_size] = '\0';
            ctx->data_size = 0;     // reset buffer
            scpi_tx_time_ticks = 0; // disable rx timeout timer
            return SCPI_RX_COMPLETE;
        } else {
            if (ctx->data_size < TERM_STR_MAX_SIZE - 1) {
                ctx->data[ctx->data_size] = (uint8_t)rx_byte;
                ctx->data_size++;
            } else {
                ctx->data_size = 0;  // wrap and overwrite in case of buffer over run
            }
        }
    }
    if (scpi_tx_time_ticks) { // check for timeout (1 second)
        if (timer_get_ticks() - scpi_tx_time_ticks > TIMER_US_TO_TICKS(1000000ULL)) {
            strcpy_P(ctx->data, PSTR(SCPI_NO_RESPONSE));
            ctx->data_size = 0;     // reset buffer
            scpi_tx_time_ticks = 0; // disable rx timeout timer
            return SCPI_RX_COMPLETE;  // this will pass the message back to the gateway
        }
    }
    return status;
}

handler_t scpi_handler = {scpi_request, scpi_response};