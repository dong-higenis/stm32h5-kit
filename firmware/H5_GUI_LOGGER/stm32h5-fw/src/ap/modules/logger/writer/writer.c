#include "writer.h"
#include "fatfs.h"

#include "common_queue.h"
#include "file_controller.h"


static bool writerThreadInit(void);
static void writerThread(void const *arg);

static void writerOpenAllFiles(void);
static void writerCloseAllFiles(void);
static void writerCheckPause(void);

static void writerCanProcess(QueueHandle_t queue, const peri_ch_info_t *p_peri);


MODULE_DEF(writer){
  .name     = "writer",
  .priority = MODULE_PRI_NORMAL,
  .init     = writerThreadInit};

/**
 * @brief 각 데이터 별 기록 파일명
 */
static const char *writer_file_tbl[PERI_MAX] =
{
  [PERI_CAN_0]   = "log/CAN0.csv",
  [PERI_CAN_1]   = "log/CAN1.csv",
  [PERI_RS485_0] = "log/RS485_0.csv",
  [PERI_RS485_1] = "log/RS485_1.csv",
  [PERI_RS232_0] = "log/RS232_0.csv",
  [PERI_LIN_0]   = "log/LIN0.csv",
  [PERI_LIN_1]   = "log/LIN1.csv",
  [PERI_LIN_2]   = "log/LIN2.csv",
  [PERI_UART_0]  = "log/UART0.csv",
  [PERI_UART_1]  = "log/UART1.csv",
};

/**
 * @brief 로그 파일 내 최상단 헤더
 */
static const char *writer_csv_header_tbl[PERI_PROTO_MAX] =
{
  [PERI_PROTO_CAN]   = "timestamp,dir,id,dlc,data,err\n",
  [PERI_PROTO_RS485] = "timestamp,dir,length,data\n",
  [PERI_PROTO_RS232] = "timestamp,dir,length,data\n",
  [PERI_PROTO_UART]  = "timestamp,dir,length,data\n",
  [PERI_PROTO_LIN]   = "timestamp,dir,id,length,data,err\n",
};

static volatile bool is_writer_paused = false;

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
  // systemWaitStart();
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

  // 2). 기록할 모든 파일을 open한다.
  writerOpenAllFiles();


  while (1)
  {
    if (is_writer_paused)
    {
      writerCheckPause();
      delay(100);
      continue; 
    }

    // peri_config에 정의된 채널 정보를 가져와 순회한다.
    for (int i = 0; i < PERI_MAX; i++)
    {
      const peri_ch_info_t *p_peri = periGetChInfo(i);

      if (p_peri == NULL)
        continue;

      QueueHandle_t queue = commonQueueGet(p_peri->name);

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

/**
 * @brief  CAN 큐에서 메시지를 꺼내 CSV 포맷 후 file_controller에 전달
 */
static void writerCanProcess(QueueHandle_t can_queue, const peri_ch_info_t *p_peri)
{
  peri_can_msg_t can_msg;

  while (xQueueReceive(can_queue, &can_msg, 0) == pdTRUE)
  {
    char     write_buffer[WRITER_LINE_MAX];
    uint16_t write_len = 0;

    // 타임스탬프, 방향, CAN ID, DLC의 길이만큼 offset을 잡는다.
    write_len = snprintf(write_buffer, sizeof(write_buffer),
                         "%lu,%s,0x%08lX,%d",
                         can_msg.timestamp,
                         (can_msg.dir == PERI_DIR_RX) ? "RX" : "TX",
                         can_msg.message.id,
                         can_msg.message.length);

    // CAN 데이터 를 기록한뒤 길이만큼 offset 증가
    for (int data_index = 0; data_index < can_msg.message.length; data_index++)
    {
      write_len += snprintf(write_buffer + write_len,
                            sizeof(write_buffer) - write_len,
                            ",%02X",
                            can_msg.message.data[data_index]);
    }

    // 에러코드와 줄바꿈을 기록한 다음 offset을 그만큼 증가
    write_len += snprintf(write_buffer + write_len,
                          sizeof(write_buffer) - write_len,
                          ",%lu\n",
                          can_msg.err_code);

    // file controller에게 파싱이 끝난 데이터 버퍼와 이름등을 전달한다.
    fileCtrlWrite(p_peri->name, write_buffer, write_len);
  }
}

/**
 * @brief  채널별 파일 경로와 CSV 헤더를 file_controller에 전달하여 파일을 연다
 */
static void writerOpenAllFiles(void)
{
  for (int i = 0; i < PERI_MAX; i++)
  {
    const peri_ch_info_t *p_peri = periGetChInfo(i);
    if (p_peri == NULL)
      continue;

    const char *file_path  = writer_file_tbl[i];
    const char *csv_header = writer_csv_header_tbl[p_peri->proto];

    fileCtrlOpen(i, file_path, csv_header);
  }
}

static void writerCloseAllFiles(void)
{
  for (int i = 0; i < PERI_MAX; i++)
  {
    fileCtrlClose((PeriName_t)i);
  }
}

/**
 * @brief  SD카드 상태를 확인하여 기록을 일시정지/재개
 *         - SD 분리 감지 → 모든 파일 닫기 + 일시정지
 *         - SD 재삽입 감지 → log 폴더 확인 + 모든 파일 다시 열기 + 재개
 */
static void writerCheckPause(void)
{
  bool sd_ok = fileCtrlCheckSD();

  if (!sd_ok && !is_writer_paused)
  {
    // SD카드 분리가 감지되면 플래그를 세워 쓰레드 루프를 멈춘다.
    writerCloseAllFiles();
    is_writer_paused = true;
  }
  else if (sd_ok && is_writer_paused)
  {
    // SD카드 재삽입 감지 -> 기존 작업으로 되돌아간다.
    if (!fatfsExist("log"))
    {
      fatfsCreateDir("log");
    }

    writerOpenAllFiles();
    is_writer_paused = false;
  }
}