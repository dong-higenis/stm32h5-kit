#include "peri.h"

static bool periThreadInit(void);
static void periThread(void const *arg);
static void periCanProcess(const peri_ch_info_t *p_peri);

/**
 * @brief peri_config에서 설정된 로깅을 위한 통신의 채널 수만큼 큐를 선언
 */
static QueueHandle_t peri_queue[PERI_MAX] = {NULL};

MODULE_DEF(peri){
  .name     = "peri",
  .priority = MODULE_PRI_NORMAL,
  .init     = periThreadInit};

QueueHandle_t periGetCanQueue(uint8_t ch)
{
  if (ch >= PERI_MAX)
    return NULL;

  return peri_queue[ch];
}

bool periThreadInit(void)
{
  bool ret = true;

  for (int i = 0; i < PERI_MAX; i++)
  {
    const peri_ch_info_t *p_peri = periGetChInfo(i);

    peri_queue[p_peri->name] = xQueueCreate(p_peri->q_depth, p_peri->q_size);
    if (peri_queue[p_peri->name] == NULL)
    {
      logPrintf("[ERR] QueueCreate failed: CAN[%d]\n", p_peri->name);
      ret = false;
      continue;
    }

    if (!canOpen(p_peri->hw_ch, CAN_NORMAL, CAN_FD_BRS, CAN_500K, CAN_500K))
    {
      logPrintf("CAN[%d] Open Failed\n", i);
      ret = false;
    }
  }

  ret &= threadCreate("peri", periThread, NULL,
                      _HW_DEF_THREAD_PERI_PRI,
                      _HW_DEF_THREAD_PERI_STACK);
  assert(ret);

  logPrintf("[%s] periThreadInit()\n", ret ? "OK" : "NG");
  return ret;
}

static void periThread(void const *arg)
{
  UNUSED(arg);

  systemWaitStart();
  logPrintf("[OK] Thread Started : PERI\n");

  while (1)
  {
    // *CAN BUS OFF등의 복구 작업을 위한 Recovery 자동화
    canUpdate();

    for (int i = 0; i < PERI_MAX; i++)
    {
      const peri_ch_info_t *p_peri = periGetChInfo(i);

      periCanProcess(p_peri);
    }

    delay(1);
  }
}

/**
 * @brief can message 파싱 및 queue push
 */
static void periCanProcess(const peri_ch_info_t *p_peri)
{
  if (p_peri == NULL)
    return;

  QueueHandle_t can_queue = periGetCanQueue(p_peri->name);
  if (can_queue == NULL)
    return;

  while (canMsgAvailable(p_peri->hw_ch))
  {
    can_msg_t can_message;
    
    //*  현재 코드 구간을 다른 실행 흐름이 끼어들지 못하게 보호한다.
    taskENTER_CRITICAL();
    canMsgRead(p_peri->hw_ch, &can_message);
    taskEXIT_CRITICAL();
    
    // * 메시지를 로그 파일 형태로 파싱한다.
    peri_can_msg_t peri_can_message = {0};

    peri_can_message.name = p_peri->name;
    peri_can_message.dir = PERI_DIR_RX;
    peri_can_message.message = can_message;
    peri_can_message.err_code = canGetError(p_peri->hw_ch);

    // 시간 기록을 위해 rtc 정보 받아오기
    rtcGetTimestamp(&peri_can_message.timestamp);

    // * rtos의 queue로 push한다.
    if (xQueueSend(can_queue, &peri_can_message, 0) != pdTRUE)
    {
      logPrintf("[ERR] CAN[%d] Queue OverFlowed, ID:0x%08X\n", p_peri->name, can_message.id);
    }
  }
}
