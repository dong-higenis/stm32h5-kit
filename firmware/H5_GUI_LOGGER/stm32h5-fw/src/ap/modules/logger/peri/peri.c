#include "peri.h"
#include "common_queue.h"

static bool periThreadInit(void);
static void periThread(void const *arg);
static void periCanProcess(const peri_ch_info_t *info);
static void periSerialProcess(const peri_ch_info_t *p_peri);

MODULE_DEF(peri){
  .name     = "peri",
  .priority = MODULE_PRI_NORMAL,
  .init     = periThreadInit};

// *기존 peri 구조체는 CAN이 포함되어 있으므로 Serial전용으로 채널을 매핑해준다.
#define SERIAL_CH_INDEX(ch) ((uint8_t)((ch) - PERI_RS485_0))

typedef struct
{
  peri_serial_msg_t serial_msg;
  bool              skip_next_lf; //* "\r\n"이 들어왔을때, '\r'로 패킷을 끝냈으니 '\n'을 무시
} serial_context_t;

static serial_context_t serial_context[SERIAL_CH_INDEX(PERI_MAX)];

bool periThreadInit(void)
{
  bool ret = true;

  // 테이블 순회하고 프로토콜별 open
  for (int i = 0; i < PERI_MAX; i++)
  {
    const peri_ch_info_t *p_peri = periGetChInfo(i);

    // 각 채널별 큐 생성
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
        
      case PERI_PROTO_RS485:
      case PERI_PROTO_RS232:
      case PERI_PROTO_UART:
        uartOpen(p_peri->hw_ch, PERI_UART_BAUD);
        break;

      
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
    // *can 통신 에러 자동 복구 호출
    canUpdate();

    for (int i = 0; i < PERI_MAX; i++)
    {
      const peri_ch_info_t *p_peri = periGetChInfo(i);

      // *통신별 처리
      switch (p_peri->proto)
      {
        case PERI_PROTO_CAN:
          periCanProcess(p_peri);
          break;

        case PERI_PROTO_RS485:
        case PERI_PROTO_RS232:
        case PERI_PROTO_UART:
          periSerialProcess(p_peri);
          break;

          // case PERI_PROTO_LIN:
          //   periLinProcess(p_peri);
          //   break;  

        default:
          break;
      }
    }

    delay(1);
  }
}

/**
 * @brief can 통신 처리
 */
static void periCanProcess(const peri_ch_info_t *p_peri)
{
  if (p_peri == NULL)
    return;

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

static bool periSerialPush(const peri_ch_info_t *p_peri,
                                 QueueHandle_t         serial_queue,
                                 serial_context_t     *p_serial)
{
  if (p_serial->serial_msg.length == 0)
    return false;

  p_serial->serial_msg.name      = p_peri->name;
  p_serial->serial_msg.dir       = PERI_DIR_RX;
  p_serial->serial_msg.timestamp = millis();

  if (xQueueSend(serial_queue, &p_serial->serial_msg, 0) != pdTRUE)
  {
    logPrintf("[ERR] %d Queue OverFlowed, Len:%d\n",
              p_peri->name,
              p_serial->serial_msg.length);
  }

  p_serial->serial_msg.length = 0;
  return true;
}

/**
 * @brief Serial 통신 처리
 */
static void periSerialProcess(const peri_ch_info_t *p_peri)
{
  if (p_peri == NULL)
    return;

  QueueHandle_t serial_queue = commonQueueGet(p_peri->name);

  if (serial_queue == NULL)
    return;

  serial_context_t *p_serial = &serial_context[SERIAL_CH_INDEX(p_peri->name)];

  while (uartAvailable(p_peri->hw_ch))
  {
    uint8_t rx_data = uartRead(p_peri->hw_ch);

    //* '\r'로 패킷을 끝낸 직후 들어오는 '\n'은 중복 구분자로 보지 않고 무시
    if (p_serial->skip_next_lf)
    {
      p_serial->skip_next_lf = false;

      if (rx_data == '\n')
      {
        continue;
      }
    }

#if PERI_UART_PACKET_USE_CR
    if (rx_data == '\r')
    {
      //* CR only / CRLF 모두에서 다음 LF 하나는 버린다
      p_serial->skip_next_lf = true;

      periSerialPush(p_peri, serial_queue, p_serial);
      continue;
    }
#endif

#if PERI_UART_PACKET_USE_LF
    if (rx_data == '\n')
    {
      //* LF 기준으로 끊을 때, 직전에 들어온 '\r'은 제거
      if (p_serial->serial_msg.length > 0)
      {
        if (p_serial->serial_msg.data[p_serial->serial_msg.length - 1] == '\r')
        {
          p_serial->serial_msg.length--;
        }
      }

      periSerialPush(p_peri, serial_queue, p_serial);
      continue;
    }
#endif

    //* 버퍼에 공간이 남아있을 경우 쌓음
    if (p_serial->serial_msg.length < PERI_SERIAL_MAX_BUFF_LEN)
    {
      p_serial->serial_msg.data[p_serial->serial_msg.length++] = rx_data;
    }

    //* 버퍼가 가득 찼을 경우 바로 queue로 push
    if (p_serial->serial_msg.length >= PERI_SERIAL_MAX_BUFF_LEN)
    {
      periSerialPush(p_peri, serial_queue, p_serial);
    }
  }
}
