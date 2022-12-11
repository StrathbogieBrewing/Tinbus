#include "slcan.h"
#include "hex.h"

slcan_error_t slcan_enframe(slcan_frame_t *slcan, const canbus_message_t *can) {
    if (can->dlc > 8) {
        return SLCAN_INVALID_DLC;
    }
    if (can->ide) {
        if (can->id > 0x1FFFFFFF) {
            return SLCAN_INVALID_ID;
        }
        if (can->rtr) {
            slcan->buffer[0] = 'R';
        } else {
            slcan->buffer[0] = 'T';
        }
        hex_from_u32(&slcan->buffer[1], 8, can->id);
        slcan->size = 9;
    } else {
        if (can->id > 0x07FF) {
            return SLCAN_INVALID_ID;
        }
        if (can->rtr) {
            slcan->buffer[0] = 'r';
        } else {
            slcan->buffer[0] = 't';
        }
        hex_from_u32(&slcan->buffer[1], 3, can->id);
        slcan->size = 4;
    }
    slcan->buffer[slcan->size] = hex_from_u4(can->dlc).value;
    slcan->size += 1;
    for (uint8_t can_index = 0; can_index < can->dlc; can_index++) {
        hex_from_u8(&slcan->buffer[slcan->size], 2, can->data[can_index]);
        slcan->size += 2;
    }
    slcan->buffer[slcan->size] = '\r'; // normal Lawicel line termination
    slcan->size += 1;
    return SLCAN_OK;
}

slcan_error_t slcan_deframe(canbus_message_t *can, const slcan_frame_t *slcan) {
    uint8_t slcan_index = 0;
    if ((slcan->buffer[0] == 'T') || (slcan->buffer[0] == 'R')) {
        hex_u32_result_t u32 = hex_to_u32(&slcan->buffer[1], 8);
        if (u32.error) {
            return SLCAN_ERROR;
        }
        can->id = u32.value;
        slcan_index = 9;
        can->ide = true;
        if (slcan->buffer[0] == 'R') {
            can->rtr = true;
        } else {
            can->rtr = false;
        }
    } else if ((slcan->buffer[0] == 't') || (slcan->buffer[0] == 'r')) {
        hex_u32_result_t u32 = hex_to_u32(&slcan->buffer[1], 3);
        if (u32.error) {
            return SLCAN_ERROR;
        }
        can->id = u32.value;
        slcan_index = 4;
        can->ide = false;
        if (slcan->buffer[0] == 'r') {
            can->rtr = true;
        } else {
            can->rtr = false;
        }
    } else {
        return SLCAN_NOT_DATA;
    }
    hex_u8_result_t u4 = hex_to_u4(slcan->buffer[slcan_index]);
    slcan_index += 1;
    if (u4.error) {
        return SLCAN_ERROR;
    }
    can->dlc = u4.value;
    for (uint8_t can_index = 0; can_index < can->dlc; can_index++) {
        hex_u8_result_t u8 = hex_to_u8(&slcan->buffer[slcan_index], 2);
        if (u8.error) {
            return SLCAN_ERROR;
        }
        can->data[can_index] = u8.value;
        slcan_index += 2;
    }
    if (slcan->buffer[slcan_index] != '\r') {
        return SLCAN_ERROR;
    }
    return SLCAN_OK;
}



// switch (uart_in[0]) {
// case 'T':
// case 't':
// case 'R':
// case 'r':
//     // Parse and transmit message
//     if (uart_in[0] < 'r') {
//         // Extended ID
//         tx.IDE = CAN_Id_Extended;
//         res = convert_hex2bin(&uart_in[1], &tx.ExtId, 8);
//         dlc_pos = 9;
//     } else {
//         tx.IDE = CAN_Id_Standard;
//         res = convert_hex2bin(&uart_in[1], &tx.StdId, 3);
//         dlc_pos = 4;
//     }
//     if (res == 0) {
//         if (uart_in[0] == 't' || uart_in[0] == 'T') {
//             tx.RTR = CAN_RTR_Data;
//             res = convert_hex2bin(&uart_in[dlc_pos], &conv_val, 1);
//             if (res == 0 && conv_val <= 8) {
//                 tx.DLC = (uint8_t)(conv_val & 0xFF);
//                 for (int32_t i = 0; i < tx.DLC; i++) {
//                     res = convert_hex2bin(&uart_in[(dlc_pos + 1) + (i * 2)], &conv_val, 2);
//                     if (res == 0) {
//                         tx.Data[i] = (uint8_t)(conv_val & 0xFF);
//                     } else {
//                         break;
//                     }
//                 }
//                 if (res != 0) {
//                     send_bell();
//                     break;
//                 }
//             } else {
//                 send_bell();
//                 break;
//             }
//         } else {
//             tx.RTR = CAN_RTR_Remote;
//         }
//         /*********** DEBUG *************/
// #ifdef __DEBUG_UART_ZZ__
//         print_usart("\r\n");
//         snprintf(dbg_buf, DBG_BUF_LEN, "CAN:\r\n");
//         dbg_print_buf();
//         snprintf(dbg_buf, DBG_BUF_LEN, "    ExtId: %08X\r\n", tx.ExtId);
//         dbg_print_buf();
//         snprintf(dbg_buf, DBG_BUF_LEN, "    StdId: %08X\r\n", tx.StdId);
//         dbg_print_buf();
//         snprintf(dbg_buf, DBG_BUF_LEN, "    RTR: %02X\r\n", tx.RTR);
//         dbg_print_buf();
//         snprintf(dbg_buf, DBG_BUF_LEN, "    DLC: %02X\r\n", tx.DLC);
//         dbg_print_buf();
//         snprintf(dbg_buf, DBG_BUF_LEN, "    Data: %02X %02X %02X %02X %02X %02X %02X %02X\r\n\r\n", tx.Data[0],
//                  tx.Data[1], tx.Data[2], tx.Data[3], tx.Data[4], tx.Data[5], tx.Data[6], tx.Data[7]);
//         dbg_print_buf();
// #endif
//         /*******************************/
//         last_mbox = CAN_Transmit(CAN1, &tx);
//         last_send_ts = millis();
//     } else {
//         send_bell();
//     }
//     break;
// case 'N':
//     // Report serial number (taken from STM32F103 ID register
//     if (uart_in[1] == S_CR || uart_in[1] == S_NL) {
//         uart_out.stream[uart_out.write][0] = 6;
//         uart_out.stream[uart_out.write][1] = 'N';
//         uart_out.stream[uart_out.write][2] = bin2hex[(SERIAL_NUM >> 12) & 0x0F];
//         uart_out.stream[uart_out.write][3] = bin2hex[(SERIAL_NUM >> 8) & 0x0F];
//         uart_out.stream[uart_out.write][4] = bin2hex[(SERIAL_NUM >> 4) & 0x0F];
//         uart_out.stream[uart_out.write][5] = bin2hex[(SERIAL_NUM)&0x0F];
//         uart_out.stream[uart_out.write][6] = S_CR;
//         uart_out.write = (uart_out.write + 1) & STREAM_UART;
//     } else {
//         // That's some gibberish, send error
//         send_bell();
//     }
//     break;
// case 'U':
//     // Set UART baud rate
//     break;
// case 'V':
//     if (uart_in[1] == S_CR || uart_in[1] == S_NL) {
//         write_usart(DB_SLCAN_VERSION, DB_SLCAN_VER_LEN);
//     } else {
//         // That's some gibberish, send error
//         send_bell();
//     }
//     break;
// case 'Z':
//     // Turn the timestamp on/off
//     if (uart_in[2] == S_CR || uart_in[2] == S_NL) {
//         if (uart_in[1] == '1') {
//             can_timestamp_on = 1;
//             send_cr();
//         } else if (uart_in[1] == '0') {
//             can_timestamp_on = 0;
//             send_cr();
//         } else {
//             // Something's not supported
//             send_bell();
//         }

//     } else {
//         send_bell();
//     }
//     break;
// default:
//     // Something's not supported (yet?)
//     send_bell();
//     break;

//     void pars_slcancmd(char *buf) { // LAWICEL PROTOCOL
//         switch (buf[0]) {
//         case 'O': // OPEN CAN
//             working = true;
//             ESP32Can.CANInit();
//             msg_cnt_in = 0;
//             msg_cnt_out = 0;
//             print_status();
//             slcan_ack();
//             break;
//         case 'C': // CLOSE CAN
//             working = false;
//             ESP32Can.CANStop();
//             print_status();
//             slcan_ack();
//             break;
//         case 't': // SEND STD FRAME
//             send_canmsg(buf, false, false);
//             slcan_ack();
//             break;
//         case 'T': // SEND EXT FRAME
//             send_canmsg(buf, false, true);
//             slcan_ack();
//             break;
//         case 'r': // SEND STD RTR FRAME
//             send_canmsg(buf, true, false);
//             slcan_ack();
//             break;
//         case 'R': // SEND EXT RTR FRAME
//             send_canmsg(buf, true, true);
//             slcan_ack();
//             break;
//         case 'Z': // ENABLE TIMESTAMPS
//             switch (buf[1]) {
//             case '0': // TIMESTAMP OFF
//                 timestamp = false;
//                 print_status();
//                 slcan_ack();
//                 break;
//             case '1': // TIMESTAMP ON
//                 timestamp = true;
//                 print_status();
//                 slcan_ack();
//                 break;
//             default:
//                 break;
//             }
//             break;
//         case 'M': /// set ACCEPTANCE CODE ACn REG
//             slcan_ack();
//             break;
//         case 'm': // set ACCEPTANCE CODE AMn REG
//             slcan_ack();
//             break;
//         case 's': // CUSTOM CAN bit-rate
//             slcan_nack();
//             break;
//         case 'S': // CAN bit-rate
//             if (working) {
//                 print_error(-1);
//                 print_status();
//                 break;
//             }
//             switch (buf[1]) {
//             case '0': // 10k
//                 print_error(10);
//                 print_status();
//                 slcan_nack();
//                 break;
//             case '1': // 20k
//                 print_error(20);
//                 print_status();
//                 slcan_nack();
//                 break;
//             case '2': // 50k
//                 print_error(50);
//                 print_status();
//                 slcan_nack();
//                 break;
//             case '3': // 100k
//                 CAN_cfg.speed = CAN_SPEED_100KBPS;
//                 can_speed = 100;
//                 print_status();
//                 slcan_ack();
//                 break;
//             case '4': // 125k
//                 CAN_cfg.speed = CAN_SPEED_125KBPS;
//                 can_speed = 125;
//                 print_status();
//                 slcan_ack();
//                 break;
//             case '5': // 250k
//                 CAN_cfg.speed = CAN_SPEED_250KBPS;
//                 can_speed = 250;
//                 print_status();
//                 slcan_ack();
//                 break;
//             case '6': // 500k
//                 CAN_cfg.speed = CAN_SPEED_500KBPS;
//                 can_speed = 500;
//                 print_status();
//                 slcan_ack();
//                 break;
//             case '7': // 800k
//                 CAN_cfg.speed = CAN_SPEED_800KBPS;
//                 can_speed = 800;
//                 print_status();
//                 slcan_ack();
//                 break;
//             case '8': // 1000k
//                 CAN_cfg.speed = CAN_SPEED_1000KBPS;
//                 can_speed = 1000;
//                 print_status();
//                 slcan_ack();
//                 break;
//             default:
//                 slcan_nack();
//                 break;
//             }
//             break;
//         case 'F': // STATUS FLAGS
//             if (bluetooth)
//                 SerialBT.print("F00");
//             else
//                 Serial.print("F00");
//             slcan_ack();
//             break;
//         case 'V': // VERSION NUMBER
//             if (bluetooth)
//                 SerialBT.print("V1234");
//             else
//                 Serial.print("V1234");
//             slcan_ack();
//             break;
//         case 'N': // SERIAL NUMBER
//             if (bluetooth)
//                 SerialBT.print("N2208");
//             else
//                 Serial.print("N2208");
//             slcan_ack();
//             break;
//         case 'l': // (NOT SPEC) TOGGLE LINE FEED ON SERIAL
//             cr = !cr;
//             slcan_nack();
//             break;
//         case 'h': // (NOT SPEC) HELP SERIAL
//             Serial.println();
//             Serial.println("mintynet.com - slcan esp32");
//             Serial.println();
//             Serial.println("O\t=\tStart slcan");
//             Serial.println("C\t=\tStop slcan");
//             Serial.println("t\t=\tSend std frame");
//             Serial.println("r\t=\tSend std rtr frame");
//             Serial.println("T\t=\tSend ext frame");
//             Serial.println("R\t=\tSend ext rtr frame");
//             Serial.println("Z0\t=\tTimestamp Off");
//             Serial.println("Z1\t=\tTimestamp On");
//             Serial.println("snn\t=\tSpeed 0xnnk N/A");
//             Serial.println("S0\t=\tSpeed 10k N/A");
//             Serial.println("S1\t=\tSpeed 20k N/A");
//             Serial.println("S2\t=\tSpeed 50k N/A");
//             Serial.println("S3\t=\tSpeed 100k");
//             Serial.println("S4\t=\tSpeed 125k");
//             Serial.println("S5\t=\tSpeed 250k");
//             Serial.println("S6\t=\tSpeed 500k");
//             Serial.println("S7\t=\tSpeed 800k");
//             Serial.println("S8\t=\tSpeed 1000k");
//             Serial.println("F\t=\tFlags N/A");
//             Serial.println("N\t=\tSerial No");
//             Serial.println("V\t=\tVersion");
//             Serial.println("-----NOT SPEC-----");
//             Serial.println("h\t=\tHelp");
//             Serial.print("l\t=\tToggle CR ");
//             if (cr) {
//                 Serial.println("ON");
//             } else {
//                 Serial.println("OFF");
//             }
//             Serial.print("CAN_SPEED:\t");
//             switch (can_speed) {
//             case 100:
//                 Serial.print("100");
//                 break;
//             case 125:
//                 Serial.print("125");
//                 break;
//             case 250:
//                 Serial.print("250");
//                 break;
//             case 500:
//                 Serial.print("500");
//                 break;
//             case 800:
//                 Serial.print("800");
//                 break;
//             case 1000:
//                 Serial.print("1000");
//                 break;
//             default:
//                 break;
//             }
//             Serial.print("kbps");
//             if (timestamp) {
//                 Serial.print("\tT");
//             }
//             if (working) {
//                 Serial.print("\tON");
//             } else {
//                 Serial.print("\tOFF");
//             }
//             Serial.println();
//             slcan_nack();
//             break;
//         default:
//             slcan_nack();
//             break;
//         }
//     } // pars_slcancmd()