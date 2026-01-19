#ifndef UI_DRAW_H
#define UI_DRAW_H

#include "ap_def.h"

void drawMainMenu(uint8_t selected);
void drawProtocolMenu(uint8_t selected);
void drawProtocolDetailMenu(uint8_t selected_peripheral,uint8_t selected_feature);
void drawChannelMenu(uint8_t selected_channel, uint8_t max_ch);

// api
void drawByteBox(int x, int y, uint8_t value);
int  getTextWidth(const char *str);
void drawSelectedBox(int x, int y, int w, int h, const char *text);
void drawNormalBox(int x, int y, int w, int h, const char *text);

#endif
