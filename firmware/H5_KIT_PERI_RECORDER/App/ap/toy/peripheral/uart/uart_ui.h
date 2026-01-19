#ifndef UART_UI_H
#define UART_UI_H

#include "ap_def.h"
#include "ui_def.h"

void uartUiDraw(const char *feature);
void uartUiOnSelect(const char *feature);
void uartUiPushBytes(uint8_t ch, uint8_t *data, uint16_t len);

#endif