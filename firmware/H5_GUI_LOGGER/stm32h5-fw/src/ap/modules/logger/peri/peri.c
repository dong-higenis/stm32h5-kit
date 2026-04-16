#include "peri.h"
#include "common_queue.h"

static bool periThreadInit(void);
static void periThread(void const *arg);
static void periCanProcess(const peri_ch_info_t *info);

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
    const peri_ch_info_t *p_peri = periGetChInfo(i);

    // 각 페리페럴에 대한 큐 생성
    if (!commonQueueCreate(p_peri->name, p_peri->q_depth, p_peri->q_size))
    {
      ret = false;
    }

    switch (p_peri->proto)
    {
      case PERI_PROTO_CAN:
        if (!canOpen(p_peri->hw_ch, CAN_NORMAL, CAN_CLASSIC, CAN_500K, CAN_500K))
        {
          logPrintf("CAN[%d] Open Failed\n", i);
        }
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
      const peri_ch_info_t *p_peri = periGetChInfo(i);

      switch (p_peri->proto)
      {
        case PERI_PROTO_CAN:
          periCanProcess(p_peri);
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
    
    taskENTER_CRITICAL();
    canMsgRead(p_peri->hw_ch, &can_message);
    taskEXIT_CRITICAL();

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
      logPrintf("[ERR] %d Queue OverFlowed, ID:0x%08X\n", p_peri->name, can_message.id);
    }
  }
}