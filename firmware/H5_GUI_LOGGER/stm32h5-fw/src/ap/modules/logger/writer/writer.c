#include "writer.h"
#include "peri_def.h"

static bool writerThreadInit(void);
static void writerThread(void const *arg);

static void writerCanProcess(uint8_t ch);

MODULE_DEF(writer)
{
  .name     = "writer",
  .priority = MODULE_PRI_NORMAL,
  .init     = writerThreadInit
};

static QueueHandle_t q_can[HW_CAN_MAX_CH];


bool writerThreadInit(void)
{
  bool ret;

  // 1). peri task의 Queue 핸들 획득
  for (int i = 0; i < HW_CAN_MAX_CH; i++)
  {
    q_can[i] = periGetCanQueue(i);
    assert(q_can[i] != NULL);
  }

  ret = threadCreate("writer", writerThread, NULL, _HW_DEF_THREAD_WRITER_PRI, _HW_DEF_THREAD_WRITER_STACK);
  assert(ret);

  logPrintf("[%s] writerThreadInit()\n", ret ? "OK" : "NG");
  return ret;
}

void writerThread(void const *arg)
{
  systemWaitStart();

  logPrintf("[OK] Thread Started : WRITER\n");

  while (1)
  {
    // CAN 채널별 Queue에서 꺼내서 SD에 쓰기
    for (int i = 0; i < HW_CAN_MAX_CH; i++)
    {
      writerCanProcess(i);
    }

    delay(1);
  }
}

static void writerCanProcess(uint8_t ch)
{
  peri_can_msg_t log_msg;

  while (xQueueReceive(q_can[ch], &log_msg, 0) == pdTRUE)
  {
    // SD카드에 쓰기
    // 예시 포맷: [timestamp] CH DIR ID DLC DATA...
    //
    // TODO: 실제 SD write 구현
    // sdWrite(&log_msg, sizeof(peri_can_msg_t));

    logPrintf("[%08u] CAN%d %s %s 0x%08X L:%02d ",
      log_msg.timestamp,
      log_msg.ch,
      (log_msg.dir == PERI_DIR_RX) ? "RX" : "TX",
      (log_msg.message.id_type == CAN_STD) ? "STD" : "EXT",
      log_msg.message.id,
      log_msg.message.length);

    for (int i = 0; i < log_msg.message.length; i++)
    {
      logPrintf("%02X ", log_msg.message.data[i]);
    }

    if (log_msg.err_code != CAN_ERR_NONE)
    {
      logPrintf("ERR:0x%X", log_msg.err_code);
    }

    logPrintf("\n");
  }
}
