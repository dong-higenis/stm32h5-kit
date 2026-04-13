#include "peri.h"

static bool periThreadInit(void);
static void periThread(void const *arg);
static void periCanProcess(const peri_ch_info_t *info);

const peri_ch_info_t peri_ch_tbl[PERI_MAX] =
{
  {PERI_CAN_0,   PERI_PROTO_CAN,   _DEF_CAN1,          "CAN0",    PERI_CAN_QUEUE_DEPTH, sizeof(peri_can_msg_t)   },
  {PERI_CAN_1,   PERI_PROTO_CAN,   _DEF_CAN2,          "CAN1",    PERI_CAN_QUEUE_DEPTH, sizeof(peri_can_msg_t)   },
  {PERI_RS485_0, PERI_PROTO_RS485, HW_UART_CH_RS485_1, "RS485_0", PERI_QUEUE_DEPTH,     sizeof(peri_serial_msg_t)},
  {PERI_RS485_1, PERI_PROTO_RS485, HW_UART_CH_RS485_2, "RS485_1", PERI_QUEUE_DEPTH,     sizeof(peri_serial_msg_t)},
  {PERI_RS232_0, PERI_PROTO_RS232, HW_UART_CH_RS232,   "RS232_0", PERI_QUEUE_DEPTH,     sizeof(peri_serial_msg_t)},
  {PERI_LIN_0,   PERI_PROTO_LIN,   HW_UART_CH_LIN_1,   "LIN0",    PERI_QUEUE_DEPTH,     sizeof(peri_lin_msg_t)   },
  {PERI_LIN_1,   PERI_PROTO_LIN,   HW_UART_CH_LIN_2,   "LIN1",    PERI_QUEUE_DEPTH,     sizeof(peri_lin_msg_t)   },
  {PERI_LIN_2,   PERI_PROTO_LIN,   HW_UART_CH_LIN_3,   "LIN2",    PERI_QUEUE_DEPTH,     sizeof(peri_lin_msg_t)   },
  {PERI_UART_0,  PERI_PROTO_UART,  HW_UART_CH_UART_1,  "UART0",   PERI_QUEUE_DEPTH,     sizeof(peri_serial_msg_t)},
  {PERI_UART_1,  PERI_PROTO_UART,  HW_UART_CH_UART_2,  "UART1",   PERI_QUEUE_DEPTH,     sizeof(peri_serial_msg_t)},
};

MODULE_DEF(peri){
  .name     = "peri",
  .priority = MODULE_PRI_NORMAL,
  .init     = periThreadInit};

bool periThreadInit(void)
{
  bool ret = true;

  // 테이블 순회하며 프로토콜별 open
  for (int i = 0; i < PERI_MAX; i++)
  {
    peri_ch_info_t *p_peri = &peri_ch_tbl[i];

    // 각 페리페럴에 대한 큐 생성
    if (!commonQueueCreate(p_peri->name, p_peri->q_depth, p_peri->q_size))
    {
      ret = false;
    }

    switch (p_peri->proto)
    {
      case PERI_PROTO_CAN:
        canOpen(p_peri->hw_ch, CAN_NORMAL, CAN_CLASSIC, CAN_500K, CAN_500K);
        break;

        // case PERI_PROTO_RS485:
        // case PERI_PROTO_RS232:
        // case PERI_PROTO_UART:
        //   uartOpen(ch->hw_ch, ...);
        //   break;

        // case PERI_PROTO_LIN:
        //   uartOpen(ch->hw_ch, ...);
        //   break;

      default:
        break;
    }
  }

  ret = threadCreate("peri", periThread, NULL,
                     _HW_DEF_THREAD_PERI_PRI,
                     _HW_DEF_THREAD_PERI_STACK);
  assert(ret);

  logPrintf("[%s] periThreadInit()\n", ret ? "OK" : "NG");
  return ret;
}

static void periThread(void const *arg)
{
  systemWaitStart();
  logPrintf("[OK] Thread Started : PERI\n");

  while (1)
  {
    canUpdate();

    for (int i = 0; i < PERI_MAX; i++)
    {
      switch (peri_ch_tbl[i].proto)
      {
        case PERI_PROTO_CAN:
          periCanProcess(&peri_ch_tbl[i]);
          break;

          // case PERI_PROTO_RS485:
          // case PERI_PROTO_RS232:
          // case PERI_PROTO_UART:
          //   periSerialProcess(&peri_ch_tbl[i]);
          //   break;

          // case PERI_PROTO_LIN:
          //   periLinProcess(&peri_ch_tbl[i]);
          //   break;

        default:
          break;
      }
    }

    delay(1);
  }
}

static void periCanProcess(const peri_ch_info_t *p_peri)
{
  QueueHandle_t can_queue = commonQueueGet(p_peri->name);
  if (can_queue == NULL)
    return;

  while (canMsgAvailable(p_peri->hw_ch))
  {
    can_msg_t can_message;
    canMsgRead(p_peri->hw_ch, &can_message);

    peri_can_msg_t peri_can_message =
    {
      .name      = p_peri->name,
      .dir       = PERI_DIR_RX,
      .timestamp = millis(),
      .message   = can_message,
      .err_code  = canGetError(p_peri->hw_ch),
    };

    if (xQueueSend(can_queue, &peri_can_message, 0) != pdTRUE)
    {
      logPrintf("[ERR] %s Queue OverFlowed, ID:0x%08X\n", p_peri->str, can_message.id);
    }
  }
}