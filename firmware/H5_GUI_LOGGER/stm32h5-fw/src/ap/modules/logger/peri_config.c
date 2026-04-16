#include "peri_config.h"

static const peri_ch_info_t peri_ch_tbl[PERI_MAX] =
{
  {PERI_CAN_0,   PERI_PROTO_CAN,   _DEF_CAN1,          PERI_CAN_QUEUE_DEPTH, sizeof(peri_can_msg_t)   },
  {PERI_CAN_1,   PERI_PROTO_CAN,   _DEF_CAN2,          PERI_CAN_QUEUE_DEPTH, sizeof(peri_can_msg_t)   },
  {PERI_RS485_0, PERI_PROTO_RS485, HW_UART_CH_RS485_1, PERI_QUEUE_DEPTH,     sizeof(peri_serial_msg_t)},
  {PERI_RS485_1, PERI_PROTO_RS485, HW_UART_CH_RS485_2, PERI_QUEUE_DEPTH,     sizeof(peri_serial_msg_t)},
  {PERI_RS232_0, PERI_PROTO_RS232, HW_UART_CH_RS232,   PERI_QUEUE_DEPTH,     sizeof(peri_serial_msg_t)},
  {PERI_LIN_0,   PERI_PROTO_LIN,   HW_UART_CH_LIN_1,   PERI_QUEUE_DEPTH,     sizeof(peri_lin_msg_t)   },
  {PERI_LIN_1,   PERI_PROTO_LIN,   HW_UART_CH_LIN_2,   PERI_QUEUE_DEPTH,     sizeof(peri_lin_msg_t)   },
  {PERI_LIN_2,   PERI_PROTO_LIN,   HW_UART_CH_LIN_3,   PERI_QUEUE_DEPTH,     sizeof(peri_lin_msg_t)   },
  {PERI_UART_0,  PERI_PROTO_UART,  HW_UART_CH_UART_1,  PERI_QUEUE_DEPTH,     sizeof(peri_serial_msg_t)},
  {PERI_UART_1,  PERI_PROTO_UART,  HW_UART_CH_UART_2,  PERI_QUEUE_DEPTH,     sizeof(peri_serial_msg_t)},
};

const peri_ch_info_t *periGetChInfo(PeriName_t name)
{
  if (name >= PERI_MAX)
    return NULL;

  return &peri_ch_tbl[name];
}

PeriProto_t periGetProto(PeriName_t name)
{
  if (name >= PERI_MAX)
    return PERI_PROTO_MAX;

  return peri_ch_tbl[name].proto;
}

uint8_t periGetHwCh(PeriName_t name)
{
  if (name >= PERI_MAX)
    return 0xFF;

  return peri_ch_tbl[name].hw_ch;
}