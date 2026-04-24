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
static bool writerCanProcess(QueueHandle_t can_queue, const peri_ch_info_t *p_peri);

MODULE_DEF(writer){
  .name     = "writer",
  .priority = MODULE_PRI_NORMAL,
  .init     = writerThreadInit};

/**
 * @brief 저장할 파일의 절대 경로
 */
static const char *writer_file_tbl[PERI_MAX] =
{
  [PERI_CAN_1] = "log/CAN1/CAN1",
  [PERI_CAN_2] = "log/CAN2/CAN2",
};

/**
 * @brief 저장할 로그파일의 첫 행
 */
static const char *writer_csv_header_tbl[PERI_PROTO_MAX] =
{
  [PERI_PROTO_CAN] = "datetime,dir,id,dlc,"
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

static uint16_t writerAppendComma(char *p_buf, uint16_t buf_size, uint16_t cur_len)
{
  if (cur_len >= buf_size)
    return 0;
  return snprintf(p_buf + cur_len, buf_size - cur_len, ",");
}

static uint16_t writerAppendHexByte(char *p_buf, uint16_t buf_size, uint16_t cur_len, uint8_t byte)
{
  if (cur_len >= buf_size)
    return 0;
  return snprintf(p_buf + cur_len, buf_size - cur_len, ",0x%02X", byte);
}

static uint16_t writerAppendErrCode(char *p_buf, uint16_t buf_size, uint16_t cur_len, uint32_t err_code)
{
  if (cur_len >= buf_size)
    return 0;
  return snprintf(p_buf + cur_len, buf_size - cur_len, ",%lu\n", err_code);
}

static bool writerCanProcess(QueueHandle_t can_queue, const peri_ch_info_t *p_peri)
{
  peri_can_msg_t can_msg;

  while (xQueueReceive(can_queue, &can_msg, 0) == pdTRUE)
  {
    char     write_buffer[WRITER_LINE_MAX];
    uint16_t write_len = 0;
    uint16_t ret;

    write_len = snprintf(write_buffer, sizeof(write_buffer),
                     "%04u-%02u-%02u %02u:%02u:%02u.%03u,%s,0x%08lX,%d",
                     can_msg.timestamp.year,
                     can_msg.timestamp.month,
                     can_msg.timestamp.day,
                     can_msg.timestamp.hour,
                     can_msg.timestamp.minute,
                     can_msg.timestamp.second,
                     can_msg.timestamp.msec,
                     (can_msg.dir == PERI_DIR_RX) ? "RX" : "TX",
                     can_msg.message.id,
                     can_msg.message.length);

    if (write_len >= sizeof(write_buffer))
      goto log_drop;

    uint8_t dlc = can_msg.message.length;
    if (dlc > WRITER_CAN_DATA_COLUMN)
      dlc = WRITER_CAN_DATA_COLUMN;

    for (int data_index = 0; data_index < dlc; data_index++)
    {
      ret = writerAppendHexByte(write_buffer, sizeof(write_buffer), write_len, can_msg.message.data[data_index]);
      if (ret == 0)
        goto log_drop;
      write_len += ret;
    }

    for (int data_index = dlc; data_index < WRITER_CAN_DATA_COLUMN; data_index++)
    {
      ret = writerAppendComma(write_buffer, sizeof(write_buffer), write_len);
      if (ret == 0)
        goto log_drop;
      write_len += ret;
    }

    ret = writerAppendErrCode(write_buffer, sizeof(write_buffer), write_len, can_msg.err_code);
    if (ret == 0)
      goto log_drop;
    write_len += ret;

    if (!fileCtrlWrite(p_peri->name, &can_msg.timestamp, write_buffer, write_len))
    {
      logPrintf("[ERR] writer line dropped: id=0x%08lX\n", can_msg.message.id);
      continue;
    }

    continue;

log_drop:
    logPrintf("[ERR] writer line dropped (buf overflow): id=0x%08lX\n", can_msg.message.id);
  }

  return true;
}

static void writerOpenAllFiles(void)
{
  fileCtrlResetLogStart();

  for (int i = 0; i < PERI_MAX; i++)
  {
    const peri_ch_info_t *p_peri = periGetChInfo(i);
    if (p_peri == NULL)
      continue;

    fileCtrlInit((PeriName_t)i, writer_file_tbl[i], writer_csv_header_tbl[p_peri->proto]);
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
