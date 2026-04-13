#include "writer.h"
#include "peri_def.h"

#include "common_queue.h"

static bool writerThreadInit(void);
static void writerThread(void const *arg);

static void writerFlushBuffer(uint8_t idx);
static void writerOpenAllFiles(void);
static void writerCloseAllFiles(void);
static void writerCheckPause(void);

static void writerCanFormat(const peri_can_msg_t *msg, char *out, uint16_t max_len, uint16_t *written);

MODULE_DEF(writer){
  .name     = "writer",
  .priority = MODULE_PRI_NORMAL,
  .init     = writerThreadInit};

bool writerThreadInit(void)
{
  bool ret;

  ret = threadCreate("writer", writerThread, NULL,
                     _HW_DEF_THREAD_WRITER_PRI,
                     _HW_DEF_THREAD_WRITER_STACK);
  assert(ret);

  logPrintf("[%s] writerThreadInit()\n", ret ? "OK" : "NG");
  return ret;
}

static void writerThread(void const *arg)
{
  systemWaitStart();
  logPrintf("[OK] Thread Started : WRITER\n");

  // 0). 파일 시스템 마운트 대기
  while (!fatfsIsMount())
  {
    delay(100);
  }

  // 1). SD카드 내 최상위 루트에서 "log" 폴더를 확인 -> 존재하지 않으면 새로 만든다.
  if (!fatfsExist("log"))
  {
    fatfsCreateDir("log");
  }
  

  writerOpenAllFiles();

  while (1)
  {
    /* peri_ch_tbl을 순회 — 채널/프로토콜 정보를 재정의하지 않음 */
    for (int i = 0; i < PERI_MAX; i++)
    {
      peri_ch_info_t *p_peri = &peri_ch_tbl[i];
      QueueHandle_t   queue  = commonQueueGet(p_peri->name);
      if (queue == NULL)
        continue;

      switch (p_peri->proto)
      {
        case PERI_PROTO_CAN:
          writerCanProcess(queue, p_peri);
          break;

          // case PERI_PROTO_RS485:
          // case PERI_PROTO_RS232:
          // case PERI_PROTO_UART:
          //   writerSerialProcess(q, ch);
          //   break;

          // case PERI_PROTO_LIN:
          //   writerLinProcess(q, ch);
          //   break;

        default:
          break;
      }
    }

    writerCheckPause();
    delay(1);
  }
}

static void writerCanProcess(QueueHandle_t can_queue, const peri_ch_info_t *p_peri)
{
  peri_can_msg_t can_msg;

  while (xQueueReceive(can_queue, &can_msg, 0) == pdTRUE)
  {
    char line[128];
    uint16_t len = 0;

    writerCanFormat(&can_msg, line, sizeof(line), &len);

    /* 채널 인덱스(info->name)로 버퍼에 적재 */
    uint8_t idx = (uint8_t)p_peri->name;

    // TODO: buf[idx]에 line 복사 후 임계 크기 도달 시 flush
    // memcpy(&buf[idx].data[buf[idx].pos], line, len);
    // buf[idx].pos += len;
    // if (buf[idx].pos >= FLUSH_THRESHOLD)
    //   writerFlushBuffer(idx);
  }
}

// static void writerCanFormat(const peri_can_msg_t *msg, char *out, uint16_t max_len, uint16_t *written)
// {
//   int n = snprintf(out, max_len,
//                    "%lu,%s,%s,0x%08lX,%d",
//                    msg->timestamp,
//                    peri_ch_tbl[msg->name].str,
//                    (msg->dir == PERI_DIR_RX) ? "RX" : "TX",
//                    msg->message.id,
//                    msg->message.length);

//   for (int i = 0; i < msg->message.length && n < max_len - 3; i++)
//   {
//     n += snprintf(out + n, max_len - n, ",%02X", msg->message.data[i]);
//   }

//   n += snprintf(out + n, max_len - n, "\n");

//   if (written)
//     *written = (uint16_t)n;
// }