#include "writer.h"
#include "fatfs.h"

#include "file_controller.h"
#include "peri.h"

static bool writerThreadInit(void);
static void writerThread(void const *arg);

static void writerOpenAllFiles(void);
static void writerCloseAllFiles(void);
static bool writerCheckPause(void);
static void writerFlushPeriodically(void);
static void writerCanProcess(QueueHandle_t queue, const peri_ch_info_t *p_peri);

MODULE_DEF(writer){
  .name     = "writer",
  .priority = MODULE_PRI_NORMAL,
  .init     = writerThreadInit};

/**
 * @brief 저장할 파일의 절대 경로
 */
static const char *writer_file_tbl[PERI_MAX] =
{
  [PERI_CAN_1] = "log/CAN1.csv",
  [PERI_CAN_2] = "log/CAN2.csv",
};

/**
 * @brief 저장할 로그파일의 첫 행
 */
static const char *writer_csv_header_tbl[PERI_PROTO_MAX] =
{
  [PERI_PROTO_CAN] = "timestamp,dir,id,dlc,"
                     "d0,d1,d2,d3,d4,d5,d6,d7,"
                     "d8,d9,d10,d11,d12,d13,d14,d15,"
                     "d16,d17,d18,d19,d20,d21,d22,d23,"
                     "d24,d25,d26,d27,d28,d29,d30,d31,"
                     "d32,d33,d34,d35,d36,d37,d38,d39,"
                     "d40,d41,d42,d43,d44,d45,d46,d47,"
                     "d48,d49,d50,d51,d52,d53,d54,d55,"
                     "d56,d57,d58,d59,d60,d61,d62,d63,"
                     "err\n",
};

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
  UNUSED(arg);

  logPrintf("[OK] Thread Started : WRITER\n");

  while (1)
  {
    // *중간에 sd카드가 빠졌는지 그 여부를 계속해서 체크한다.
    bool sd_ok = writerCheckPause();

    // *모든 통신 채널에 대한 큐를 받아온다.
    for (int i = 0; i < PERI_MAX; i++)
    {
      const peri_ch_info_t *p_peri = periGetChInfo(i);
      if (p_peri == NULL)
        continue;

      QueueHandle_t queue = periGetCanQueue(p_peri->name);
      if (queue == NULL)
        continue;

      if (sd_ok)
      {
        writerCanProcess(queue, p_peri);
      }
      else
      {
        // *마운트되지 않은 동안에는 큐를 비워 overflow를 방지한다.
        peri_can_msg_t can_msg;
        while (xQueueReceive(queue, &can_msg, 0) == pdTRUE)
        {
        }
      }
    }

    if (sd_ok)
    {
      // *주기적으로 버퍼가 가득 차있지 않아도 sd카드에 데이터를 밀어넣어 준다.
      writerFlushPeriodically();
      delay(1);
    }
    else
    {
      delay(100);
    }
  }
}

static void writerCanProcess(QueueHandle_t can_queue, const peri_ch_info_t *p_peri)
{
  peri_can_msg_t can_msg;

  // * 큐에 쌓인 CAN 로그를 하나씩 꺼내 CSV 한 줄로 변환한다.
  while (xQueueReceive(can_queue, &can_msg, 0) == pdTRUE)
  {
    // * 한 줄 전체를 임시로 조립할 버퍼와 현재 길이를 준비한다.
    char     write_buffer[WRITER_LINE_MAX];
    uint16_t write_len = 0;

    // * timestamp, dir, id, dlc 까지 CSV 앞부분을 먼저 작성한다.
    write_len = snprintf(write_buffer, sizeof(write_buffer),
                         "%lu,%s,0x%08lX,%d",
                         can_msg.timestamp,
                         (can_msg.dir == PERI_DIR_RX) ? "RX" : "TX",
                         can_msg.message.id,
                         can_msg.message.length);

    // * 실제 DLC 길이만큼 CAN data 바이트를 뒤에 이어 붙인다.
    for (int data_index = 0; data_index < can_msg.message.length; data_index++)
    {
      write_len += snprintf(write_buffer + write_len,
                            sizeof(write_buffer) - write_len,
                            ",0x%02X",
                            can_msg.message.data[data_index]);
    }

    // * 남은 data 컬럼은 비워서 CSV 열 개수를 항상 동일하게 맞춘다.
    for (int data_index = can_msg.message.length; data_index < WRITER_CAN_DATA_COLUMN; data_index++)
    {
      write_len += snprintf(write_buffer + write_len,
                            sizeof(write_buffer) - write_len,
                            ",");
    }

    // * 마지막 컬럼에 에러 코드를 추가하고 한 줄을 마무리한다.
    write_len += snprintf(write_buffer + write_len,
                          sizeof(write_buffer) - write_len,
                          ",%lu\n",
                          can_msg.err_code);

    // * 파일 관리자에게 쓰기 요청을 지시한다.
    fileCtrlWrite(p_peri->name, write_buffer, write_len);
  }
}

static void writerOpenAllFiles(void)
{
  // *로그 기록 시작에 대한 cli 출력 허용
  fileCtrlResetLogStart();

  for (int i = 0; i < PERI_MAX; i++)
  {
    const peri_ch_info_t *p_peri = periGetChInfo(i);
    if (p_peri == NULL)
      continue;

    // *파일 관리자에게 열려고 하는 파일 정보를 전달한다.
    fileCtrlOpen((PeriName_t)i, writer_file_tbl[i], writer_csv_header_tbl[p_peri->proto]);
  }
}

static void writerCloseAllFiles(void)
{
  for (int i = 0; i < PERI_MAX; i++)
  {
    // *모든 파일들을 닫는다.
    fileCtrlClose((PeriName_t)i);
  }
}

/**
 * @brief 주기적으로 writing을 요청하는 함수
 */
static void writerFlushPeriodically(void)
{
  static uint32_t pre_time = 0;

  if (millis() - pre_time < WRITER_FORCE_FLUSH_PERIOD_MS)
    return;

  fileCtrlFlushAll();
  pre_time = millis();
}

/**
 * @brief loop내에서 파일시스템의 마운트 여부를 감지하고,
 *        파일을 열고 닫고 등을 담당하는 함수
 */
static bool writerCheckPause(void)
{
  static bool prev_mount_state = false;
  bool        sd_ok            = fatfsIsMount();

  // *상태가 변하지 않았으면 return
  if (sd_ok == prev_mount_state)
    return sd_ok;

  // *변화된 상태에 따라 파일을 열거나 닫는다.
  if (!sd_ok)
  {
    writerCloseAllFiles();
  }
  else
  {
    // *log 폴더가 없다면 생성
    if (!fatfsExist("log"))
    {
      fatfsCreateDir("log");
    }

    writerOpenAllFiles();
  }

  prev_mount_state = sd_ok;

  return sd_ok;
}
