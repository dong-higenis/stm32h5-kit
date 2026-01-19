#ifndef UI_DEF_H
#define UI_DEF_H

#include "ap_def.h"

typedef struct
{
  const char  *peri_name;
  const char **button_list;
  uint8_t      button_count;
  uint8_t      ch_max;
} peri_detail_t;

typedef enum
{
  L1_PERIPHERAL,
  L1_TIME,
  L1_UI_MAX
} ui_layer_1_t;

typedef enum
{
  L2_CAN,
  L2_RS485,
  L2_RS232,
  L2_UART,
  L2_LIN,
  L2_RETURN, // 이전 화면 버튼
  L2_UI_MAX
} ui_layer_2_t;

#define CAN_SEL_MAX_CH   HW_CAN_MAX_CH
#define RS485_SEL_MAX_CH 2
#define RS232_SEL_MAX_CH 1
#define UART_SEL_MAX_CH  2
#define LIN_SEL_MAX_CH   3

#define RS485_DEBUG 1

#endif
