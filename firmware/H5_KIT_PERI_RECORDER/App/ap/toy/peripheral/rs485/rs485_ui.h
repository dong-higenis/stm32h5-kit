#ifndef RS485_UI_H
#define RS485_UI_H

#include "ap_def.h"
#include "ui_def.h"

void rs485UiDraw(const char *feature);
void rs485UiOnSelect(const char *feature);
void rs485UiPushBytes(uint8_t ch, uint8_t *data, uint16_t len);

#endif
