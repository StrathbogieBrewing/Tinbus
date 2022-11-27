
		switch(uart_in[0]) {
			case 'T':
			case 't':
			case 'R':
			case 'r':
				// Parse and transmit message
				if (uart_in[0] < 'r') {
					// Extended ID
					tx.IDE = CAN_Id_Extended;
					res = convert_hex2bin(&uart_in[1], &tx.ExtId, 8);
					dlc_pos = 9;
				} else {
					tx.IDE = CAN_Id_Standard;
					res = convert_hex2bin(&uart_in[1], &tx.StdId, 3);
					dlc_pos = 4;
				}
				if (res == 0) {
					if (uart_in[0] == 't' || uart_in[0] == 'T') {
						tx.RTR = CAN_RTR_Data;
						res = convert_hex2bin(&uart_in[dlc_pos], &conv_val, 1);
						if (res == 0 && conv_val <= 8) {
							tx.DLC = (uint8_t) (conv_val & 0xFF);
							for (int32_t i = 0; i < tx.DLC; i++) {
								res = convert_hex2bin(&uart_in[(dlc_pos + 1)+(i*2)], &conv_val, 2);
								if (res == 0) {
									tx.Data[i] = (uint8_t) (conv_val & 0xFF);
								} else {
									break;
								}
							}
							if (res != 0) {
								send_bell();
								break;
							}
						} else {
							send_bell();
							break;
						}
					} else {
						tx.RTR = CAN_RTR_Remote;
					}
					/*********** DEBUG *************/
#ifdef __DEBUG_UART_ZZ__
					print_usart("\r\n");
					snprintf(dbg_buf, DBG_BUF_LEN, "CAN:\r\n");
					dbg_print_buf();
					snprintf(dbg_buf, DBG_BUF_LEN, "    ExtId: %08X\r\n", tx.ExtId);
					dbg_print_buf();
					snprintf(dbg_buf, DBG_BUF_LEN, "    StdId: %08X\r\n", tx.StdId);
					dbg_print_buf();
					snprintf(dbg_buf, DBG_BUF_LEN, "    RTR: %02X\r\n", tx.RTR);
					dbg_print_buf();
					snprintf(dbg_buf, DBG_BUF_LEN, "    DLC: %02X\r\n", tx.DLC);
					dbg_print_buf();
					snprintf(dbg_buf, DBG_BUF_LEN, "    Data: %02X %02X %02X %02X %02X %02X %02X %02X\r\n\r\n",
							tx.Data[0], tx.Data[1], tx.Data[2], tx.Data[3], tx.Data[4], tx.Data[5], tx.Data[6], tx.Data[7]);
					dbg_print_buf();
#endif
					/*******************************/
						last_mbox = CAN_Transmit(CAN1, &tx);
						last_send_ts = millis();
				} else {
					send_bell();
				}
			break;
			case 'N':
				// Report serial number (taken from STM32F103 ID register
				if (uart_in[1] == S_CR || uart_in[1] == S_NL) {
					uart_out.stream[uart_out.write][0] = 6;
					uart_out.stream[uart_out.write][1] = 'N';
					uart_out.stream[uart_out.write][2] = bin2hex[(SERIAL_NUM >> 12) & 0x0F];
					uart_out.stream[uart_out.write][3] = bin2hex[(SERIAL_NUM >> 8) & 0x0F];
					uart_out.stream[uart_out.write][4] = bin2hex[(SERIAL_NUM >> 4) & 0x0F];
					uart_out.stream[uart_out.write][5] = bin2hex[(SERIAL_NUM) & 0x0F];
					uart_out.stream[uart_out.write][6] = S_CR;
					uart_out.write = (uart_out.write + 1) & STREAM_UART;
				} else {
					// That's some gibberish, send error
					send_bell();
				}
			break;
			case 'U':
				// Set UART baud rate
			break;
			case 'V':
				if (uart_in[1] == S_CR || uart_in[1] == S_NL) {
					write_usart(DB_SLCAN_VERSION, DB_SLCAN_VER_LEN);
				} else {
					// That's some gibberish, send error
					send_bell();
				}
			break;
			case 'Z':
				// Turn the timestamp on/off
				if (uart_in[2] == S_CR || uart_in[2] == S_NL) {
					if (uart_in[1] == '1') {
						can_timestamp_on = 1;
						send_cr();
					} else if (uart_in[1] == '0') {
						can_timestamp_on = 0;
						send_cr();
					} else {
						// Something's not supported
						send_bell();
					}

				} else {
					send_bell();
				}
			break;
			default:
				// Something's not supported (yet?)
				send_bell();
			break;