#include "peri.h"
#include "peri_def.h"

static bool periThreadInit(void);
static void periThread(void const *arg);

static void periCanProcess(uint8_t ch);

// 큐 핸들러
static QueueHandle_t task_queue_CAN[HW_CAN_MAX_CH];

MODULE_DEF(peri){
  .name     = "peri",
  .priority = MODULE_PRI_NORMAL,
  .init     = periThreadInit};

bool periThreadInit(void)
{
  bool ret;

  // 0). Queue 생성
  for (int i = 0; i < HW_CAN_MAX_CH; i++)
  {
    task_queue_CAN[i] = xQueueCreate(PERI_CAN_QUEUE_DEPTH, sizeof(peri_can_msg_t));
    assert(task_queue_CAN[i] != NULL);
  }

  // 1). can open
  for (int i = 0; i < HW_CAN_MAX_CH; i++)
  {
    canOpen(i, CAN_NORMAL, CAN_CLASSIC, CAN_500K, CAN_500K);
  }

  // 2). 485,232,uart,LIN

  ret = threadCreate("peri", periThread, NULL, _HW_DEF_THREAD_PERI_PRI, _HW_DEF_THREAD_PERI_STACK);
  assert(ret);

  logPrintf("[%s] periThreadInit()\n", ret ? "OK" : "NG");
  return ret;
}

void periThread(void const *arg)
{
  bool init_ret = true;

  systemWaitStart();

  logPrintf("[%s] Thread Started : PERI\n", init_ret ? "OK" : "NG");

  while (1)
  {
    canUpdate();

    // 1). CAN 채널별 수신 --> Queue Push
    for (int i = 0; i < HW_CAN_MAX_CH; i++)
    {
      periCanProcess(i);
    }

    delay(1);
  }
}

// getter
QueueHandle_t periGetCanQueue(uint8_t ch)
{
  if (ch >= HW_CAN_MAX_CH) 
    return NULL;
    
  return task_queue_CAN[ch];
}

static void periCanProcess(uint8_t ch)
{
  while (canMsgAvailable(ch))
  {
    can_msg_t can_msg;
    canMsgRead(ch, &can_msg);

    peri_can_msg_t log_msg =
    {
      .ch        = ch,
      .dir       = PERI_DIR_RX,
      .timestamp = millis(),
      .err_code  = canGetError(ch),
      .message   = can_msg,
    };
    
    // queue에 받은 메시지 push
    if (xQueueSend(task_queue_CAN[ch], &log_msg, 0) != pdTRUE)
    {
      logPrintf("[WRN] CAN%d Q full, ID:0x%08X\n", ch, can_msg.id);
    }
  }
}