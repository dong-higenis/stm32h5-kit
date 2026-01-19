#ifndef UART_APP_H
#define UART_APP_H

#include "ap_def.h"
#include "ui_def.h"

#define UART_MAX_DATA 64

void uartAppInit(void);
void uartAppOnRx(uint8_t ch);
uint32_t uartAppGetSavedNum(uint8_t ch);

void uartLogOpen(uint8_t ch);
bool uartLogWrite(uint8_t ch, const void *buf, uint32_t len);
void uartLogClose(uint8_t ch);
#endif  
