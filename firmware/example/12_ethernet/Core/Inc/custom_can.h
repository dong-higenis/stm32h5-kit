#ifndef CUSTOM_CAN_H
#define CUSTOM_CAN_H

#include <stdbool.h> 
#include <stdint.h>
#include <string.h>
#include "main.h"  // loopback 사용 여부 참조 
#include <fdcan.h> // fdcan 자동생성 파일 include
#include "usart.h" // uart1 extern을 위해 
#include "cli.h"

extern uint8_t debug_uart_rx_char;
extern bool    can_block_mode_req;

bool customCanInit(FDCAN_HandleTypeDef *can_handler, bool is_loopback);
void customCanClose(FDCAN_HandleTypeDef *can_handler);
void cliCan(cli_args_t *args);

#endif
