#include "file_controller.h"
#include "cli.h"
#include "fatfs.h"

typedef struct
{
  FIL  file_handler;                            // fatfs 파일 핸들러
  bool is_open;                                 // 파일 open 여부

  char     write_buf[FILE_CTRL_BUF_SIZE];       // explain : 써야할 데이터 보관소
  uint16_t write_length;                        // explain : 써야할 데이터 길이

  char dir_path[LOG_FILE_MAX_PATH_LEN];         // ex: "log/CAN1"
  char file_prefix[16];                         // ex: "CAN1"
  char opened_file_path[LOG_FILE_MAX_PATH_LEN]; // ex: "log/CAN1/CAN1_20260424_153012.csv"

  const char *csv_header;                       // ex: dateTime,can_id....
  uint32_t    file_opened_ms;                   // 파일이 열린 순간 기록
} file_tbl_t;

static void fileCtrlFlushForce(PeriName_t peri_name);

__attribute__((section(".non_cache"))) static file_tbl_t file_tbl[PERI_MAX];

static bool is_log_start_notified[PERI_MAX] = {
  false,
};

/**
 * @brief 파일 관리자 Init
 * @details 파일이 생성되고 저장되고, 기본적인 기능을 초기화하는 작업

 */
bool fileCtrlInit(PeriName_t name, const char *file_path, const char *csv_header)
{
  // 넘겨 받은 경로값에서 /의 마지막 위치 주소를 알기 위해 선언
  char *p_last_slash;

  if (name >= PERI_MAX || file_path == NULL)
    return false;

  // * 1). 동기화를 위해 열려고 하는 파일의 헤더 정보를 갱신한다.
  file_tbl[name].csv_header = csv_header;

  // * 1-1). 원본 경로명은 수정할 수 없으니 구조체 내부 버퍼에 복사한다.
  memset(file_tbl[name].dir_path, 0, sizeof(file_tbl[name].dir_path));
  snprintf(file_tbl[name].dir_path, sizeof(file_tbl[name].dir_path), "%s", file_path);

  // * 2). 넘겨받은 경로에서 마지막 "/(slash)"의 위치를 찾는다.
  p_last_slash = strrchr(file_tbl[name].dir_path, '/');

  // * 3). "/(slash)"가 없다면 예외 상황이므로 false를 반환한다.
  if (p_last_slash == NULL)
    return false;

  // * 4). 마지막 "/(slash)" 뒤쪽 문자열을 파일 prefix로 저장한다. (ex: log/CAN1/CAN1 -> CAN1)
  memset(file_tbl[name].file_prefix, 0, sizeof(file_tbl[name].file_prefix));
  snprintf(file_tbl[name].file_prefix, sizeof(file_tbl[name].file_prefix), "%s", p_last_slash + 1);

  // * 5). 마지막 "/(slash)"자리에 널 종료 문자를 넣어 문자열 끝을 알리고, 순수 경로만을 남긴다. (ex: log/CAN1/CAN1 -> log/CAN1)
  // * 5-1). 이 시점을 기준으로 file_tbl내 변수에 "log/CAN1", "CAN1" 두가지 문자열로 나뉘어 저장된다.
  *p_last_slash = '\0';

  // * 6). 해당 경로까지 폴더가 생성되어있지 않다면, 생성한다.
  if (!fatfsExist(file_tbl[name].dir_path))
  {
    if (!fatfsCreateDir(file_tbl[name].dir_path))
      return false;
  }

  // * 7). 아직 실제 fatfs 파일을 open하기 이전이므로, is_open을 false로 둔다.
  file_tbl[name].is_open = false;

  // * 8). 쓰기 버퍼의 잔존 데이터 수를 0으로 초기화한다.
  file_tbl[name].write_length = 0;

  // * 9). 열려있는 파일은 아직 없으므로 경로 버퍼와 열린 시간을 초기화 해준다.
  memset(file_tbl[name].opened_file_path, 0, sizeof(file_tbl[name].opened_file_path));
  memset(&file_tbl[name].file_opened_ms, 0, sizeof(file_tbl[name].file_opened_ms));

  // * 10). 기록 시작 안내는 아직 필요없으니 false로 초기화한다.
  is_log_start_notified[name] = false;

  file_tbl[name].file_opened_ms = 0;

  return true;
}

void fileCtrlResetLogStart(void)
{
  for (int i = 0; i < PERI_MAX; i++)
  {
    is_log_start_notified[i] = false;
  }
}

bool fileCtrlOpen(PeriName_t name, const rtc_timestamp_t *p_msg_time)
{
  bool is_new_file;

  if (name >= PERI_MAX || p_msg_time == NULL)
    return false;

  // * 1). 이미 파일이 열려 있다면, 파일을 연 시점부터 LOG_ROTATE_INTERVAL_MIN 만큼 지났는지 확인한다.
  if (file_tbl[name].is_open)
  {
    // * 1-1). 아직 새로운 파일로 변경할 시간이 지나지 않았다면 현재 열려 있는 파일을 그대로 사용한다. (단위 : 분단위)
    if ((millis() - file_tbl[name].file_opened_ms) < (LOG_ROTATE_INTERVAL_MIN * 60UL * 1000UL))
    {
      return true;
    }

    // * 2-2). 로테이션 시간이 지났다면 기존 파일을 닫고, 아래에서 새 파일을 열도록 한다.
    fileCtrlClose(name);
  }

  // * 3). 이번 메시지의 RTC timestamp를 사용해서 새 로그 파일명을 만든다.
  //      ex: "log/CAN1/CAN1_20260424_153012.csv"
  {
    char new_file_path[LOG_FILE_MAX_PATH_LEN];

    if (snprintf(new_file_path, sizeof(new_file_path),
                 "%s/%s_%04u%02u%02u_%02u%02u%02u.csv",
                 file_tbl[name].dir_path,
                 file_tbl[name].file_prefix,
                 p_msg_time->year,
                 p_msg_time->month,
                 p_msg_time->day,
                 p_msg_time->hour,
                 p_msg_time->minute,
                 p_msg_time->second) >= sizeof(new_file_path))
    {
      logPrintf("[ERR] fileCtrlOpen snprintf fail: %s/%s\n",
                file_tbl[name].dir_path, file_tbl[name].file_prefix);
      return false;
    }

    snprintf(file_tbl[name].opened_file_path,
             sizeof(file_tbl[name].opened_file_path),
             "%s",
             new_file_path);
  }

  // * 4). 같은 파일명이 이미 존재하는지 확인한다.
  //      새 파일이면 CSV header를 기록해야 하므로 미리 구분해 둔다.
  is_new_file = !fatfsExist(file_tbl[name].opened_file_path);

  // * 5). 실제로 파일을 열고, 실패시 false를 return한다.
  if (!fatfsOpen(&file_tbl[name].file_handler,
                 file_tbl[name].opened_file_path,
                 FA_OPEN_APPEND | FA_WRITE))
  {
    logPrintf("[ERR] fileCtrlOpen fatfsOpen fail: %s\n", file_tbl[name].opened_file_path);
    file_tbl[name].is_open = false;
    return false;
  }

  // * 6).  파일 open이 성공했으므로 현재 채널의 파일 상태를 갱신한다.
  file_tbl[name].is_open      = true;
  file_tbl[name].write_length = 0;

  // * 7). 파일이 열린 시간을 기록한다.
  file_tbl[name].file_opened_ms = millis();

  // * 8). 새 파일이라면 CSV header를 한 번 기록한다.
  if (is_new_file && file_tbl[name].csv_header != NULL)
  {
    uint32_t header_len = strlen(file_tbl[name].csv_header);

    if (!fatfsWrite(&file_tbl[name].file_handler,
                    (uint8_t *)file_tbl[name].csv_header,
                    header_len))
    {
      fatfsClose(&file_tbl[name].file_handler);
      file_tbl[name].is_open             = false;
      file_tbl[name].opened_file_path[0] = '\0';
      return false;
    }

    // * 8-1). header는 파일 생성 직후 바로 저장되도록 sync 한다.
    fatfsSync(&file_tbl[name].file_handler);
  }

  // * 9). 실제 로그 데이터가 flush될 때 "logging started" 안내를 다시 한 번 출력할 수 있도록 초기화한다.
  is_log_start_notified[name] = false;
  logPrintf("[OK] fileCtrl opened: %s\n", file_tbl[name].opened_file_path);

  return true;
}

void fileCtrlClose(PeriName_t peri_name)
{
  if (peri_name >= PERI_MAX)
    return;

  file_tbl_t *p_file = &file_tbl[peri_name];

  // *해당 파일이 열려있지 않다면 예외 상황이므로 return한다.
  if (!p_file->is_open)
    return;

  // *아직 써야할 즉 "write buffer"에 남아있는 데이터가 있다면 파일에 강제로 기록한다.
  if (p_file->write_length > 0) 
  {
    fileCtrlFlushForce(peri_name);
  }

  // *FATFS 파일 핸들러를 닫는다.
  fatfsClose(&p_file->file_handler);
  p_file->is_open = false;

  // *닫은 파일 경로를 로그로 출력한다.
  logPrintf("[OK] fileCtrl closed: %s\n", p_file->opened_file_path);
}

bool fileCtrlWrite(PeriName_t name, const rtc_timestamp_t *p_time, const char *p_data, uint16_t length)
{
  file_tbl_t *p_file;

  if (name >= PERI_MAX || p_time == NULL || p_data == NULL || length == 0)
    return false;

  // *1). 현재 파일이 열려있는지 확인 한다.
  if (!fileCtrlOpen(name, p_time))
    return false;

  p_file = &file_tbl[name];

  // *2). 새 데이터를 버퍼에 넣기 전에 공간이 부족한지 확인한다.
  if (p_file->write_length + length > FILE_CTRL_BUF_SIZE)
  {
    // *2-1). 공간이 부족하면 먼저 남아있는 데이터를 강제로 파일에 기록한다.
    fileCtrlFlushForce(name);

    // *2-2). 그래도 부족하면 한줄 자체가 버퍼 크기보다 큰 메시지이므로 실패로 간주한다.
    if (p_file->write_length + length > FILE_CTRL_BUF_SIZE)
    {
      logPrintf("[ERR] fileCtrlWrite no space: %s, len:%d\n", p_file->opened_file_path, length);
      return false;
    }
  }

  // *3). write buffer의 현재 끝 위치에 새 데이터를 복사한다.
  memcpy(&p_file->write_buf[p_file->write_length], p_data, length);

  // *4). 버퍼에 쌓인 데이터 길이를 갱신한다.
  p_file->write_length += length;

  // *5). 버퍼에 쌓인 데이터가 flush 기준 크기 이상이면 파일에 기록한다.
  if (p_file->write_length >= FILE_CTRL_FLUSH_THRESHOLD)
  {
    fileCtrlFlush(name);
  }

  return true;
}

void fileCtrlFlush(PeriName_t peri_name)
{
  uint16_t write_size;
  uint16_t remain;

  // * 1). 유효하지 않은 채널 번호라면 종료한다.
  if (peri_name >= PERI_MAX)
    return;

  // * 2). 파일이 열려 있지 않거나, 버퍼에 쓸 데이터가 없다면 종료한다.
  if (!file_tbl[peri_name].is_open || file_tbl[peri_name].write_length == 0)
    return;

  // * 3). 현재 버퍼 길이에서 FILE_CTRL_FLUSH_THRESHOLD 단위로 딱 떨어지는 만큼만 계산한다.
  // *     예: threshold가 2048일 때, 3000byte가 쌓여 있으면 2048byte만 먼저 기록한다.
  write_size = (file_tbl[peri_name].write_length / FILE_CTRL_FLUSH_THRESHOLD) * FILE_CTRL_FLUSH_THRESHOLD;

  if (write_size > 0)
  {
    // * 4). 계산된 크기만큼 FatFS에 기록한다.
    if (fatfsWrite(&file_tbl[peri_name].file_handler, (uint8_t *)file_tbl[peri_name].write_buf, write_size))
    {
      // * 4-1). 실제 로그 데이터가 처음 기록된 순간에만 logging started 메시지를 출력한다.
      if (!is_log_start_notified[peri_name])
      {
        logPrintf("[LOG] CAN logging started: %s\n", file_tbl[peri_name].opened_file_path);
        is_log_start_notified[peri_name] = true;
      }

      // * 4-2). 쓰기 성공 시에만 남은 데이터 처리
      remain = file_tbl[peri_name].write_length - write_size;

      if (remain > 0)
      {
        memmove(file_tbl[peri_name].write_buf, file_tbl[peri_name].write_buf + write_size, remain);
      }

      file_tbl[peri_name].write_length = remain;
    }
    else
    {
      // * 4-3). FatFS 쓰기에 실패했다면 버퍼를 유지하고 에러 로그만 출력한다.
      logPrintf("[ERR] fileCtrl flush failed: %s\n", file_tbl[peri_name].opened_file_path);
    }
  }
}

void fileCtrlFlushAll(void)
{
  for (int i = 0; i < PERI_MAX; i++)
  {
    if (file_tbl[i].write_length > 0)
    {
      fileCtrlFlushForce((PeriName_t)i);
    }
  }
}

static void fileCtrlFlushForce(PeriName_t peri_name)
{
  uint16_t write_size;

  if (peri_name >= PERI_MAX)
    return;

  if (!file_tbl[peri_name].is_open || file_tbl[peri_name].write_length == 0)
    return;

  write_size = file_tbl[peri_name].write_length;

  if (fatfsWrite(&file_tbl[peri_name].file_handler, (uint8_t *)file_tbl[peri_name].write_buf, write_size))
  {
    if (!is_log_start_notified[peri_name])
    {
      logPrintf("[LOG] CAN logging started: %s\n", file_tbl[peri_name].opened_file_path);
      is_log_start_notified[peri_name] = true;
    }

    fatfsSync(&file_tbl[peri_name].file_handler);
    file_tbl[peri_name].write_length = 0;
  }
  else
  {
    logPrintf("[ERR] fileCtrl force flush failed: %s\n", file_tbl[peri_name].opened_file_path);
  }
}