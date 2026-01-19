#ifndef UI_H
#define UI_H

#include "ap_def.h"
#include "ui_def.h"

typedef enum 
{
  UI_MAIN_LAYER_MENU,
  UI_MAIN_LAYER_TIME,
  UI_MAIN_LAYER_PROTOCOL,
  UI_MAIN_LAYER_PROTOCOL_DETAIL,
  UI_MAIN_LAYER_PROTOCOL_CHANNEL_SELECT,
  UI_MAIN_LYAER_PROTOCOL_FEATURE
} ui_main_layer_t;

typedef struct 
{
  ui_main_layer_t main_layer;
  uint8_t l1_index;
  uint8_t l2_index;
  uint8_t l3_index;
  uint8_t ch_index;
} ui_context_t; 


void uiInit(void);
void uiOnButton(uint32_t id);
void uiDraw(void);

const ui_context_t *uiGet(void); // 현재 선택 상태

#endif
