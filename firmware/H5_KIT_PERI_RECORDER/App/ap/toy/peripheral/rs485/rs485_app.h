#ifndef RS485_APP_H
#define RS485_APP_H

#include "ap_def.h"
#include "ui_def.h"

void rs485AppInit(void);
void rs485AppOnRx(uint8_t ui_ch);
uint32_t rs485AppGetSavedNum(uint8_t ch);

void rs485LogOpen(uint8_t ch);
bool rs485LogWrite(uint8_t ch, const void *buf, uint32_t len);
void rs485LogClose(uint8_t ch);
#endif
