#include "file_controller.h"
#include "cli.h"
#include "fatfs.h"

typedef struct
{
  FIL         file_handler;
  bool        is_open;
  char        write_buf[FILE_CTRL_BUF_SIZE];
  uint16_t    write_length;
  const char *file_path;
} file_tbl_t;

static void fileCtrlFlushForce(PeriName_t peri_name);

__attribute__((section(".non_cache"))) static file_tbl_t file_tbl[PERI_MAX];
static bool is_log_start_notified[PERI_MAX] = {false,};

bool fileCtrlOpen(PeriName_t name, const char *file_path, const char *csv_header)
{
  // * 유효하지 않은 채널 번호이거나 파일 경로가 없으면 열 수 없다.
  if (name >= PERI_MAX || file_path == NULL)
    return false;

  file_tbl_t *p_file = &file_tbl[name];

  // * 파일이 없던 상태에서 처음 생성되는 경우에만 CSV 헤더를 기록한다.
  bool is_new_file = !fatfsExist(file_path);

  // * 기존 파일이면 append, 새 파일이면 생성 후 append 모드로 연다.
  if (!fatfsOpen(&p_file->file_handler, file_path, FA_WRITE | FA_OPEN_APPEND))
  {
    p_file->is_open = false;
    return false;
  }

  // * open 성공 후 내부 상태를 초기화한다.
  p_file->is_open      = true;
  p_file->write_length = 0;
  p_file->file_path    = file_path;

  // * 새 파일일 때만 CSV 첫 줄 헤더를 한 번 기록하고 즉시 sync 한다.
  if (is_new_file && csv_header != NULL)
  {
    if (!fatfsWrite(&p_file->file_handler, (uint8_t *)csv_header, strlen(csv_header)))
    {
      fatfsClose(&p_file->file_handler);
      p_file->is_open = false;
      logPrintf("[ERR] fileCtrl header write failed: %s\n", file_path);
      return false;
    }

    fatfsSync(&p_file->file_handler);
  }

  logPrintf("[OK] fileCtrl opened: %s\n", p_file->file_path);

  return true;
}

void fileCtrlResetLogStart(void)
{
  for (int i = 0; i < PERI_MAX; i++)
  {
    is_log_start_notified[i] = false;
  }
}

void fileCtrlClose(PeriName_t peri_name)
{
  if (peri_name >= PERI_MAX)
    return;

  file_tbl_t *p_file = &file_tbl[peri_name];

  if (!p_file->is_open)
    return;

  // * 써야할 남은 데이터가 있다면 모두 쓴다.
  if (p_file->write_length > 0)
  {
    fileCtrlFlushForce(peri_name);
  }

  // * 파일을 닫아준다.
  fatfsClose(&p_file->file_handler);
  p_file->is_open = false;

  logPrintf("[OK] fileCtrl closed: %s\n", p_file->file_path);
}

void fileCtrlWrite(PeriName_t peri_name, const char *p_data, uint16_t length)
{
  if (peri_name >= PERI_MAX || p_data == NULL || length == 0)
    return;

  file_tbl_t *p_file = &file_tbl[peri_name];

  // * 파일이 열려있는지 그 여부를 파악한다.
  if (!p_file->is_open)
    return;

  // * 만약 새 데이터를 버퍼에 넣기 전에, 공간이 부족하면 먼저 한 번 flush해서 자리를 만들어 준다.
  if (p_file->write_length + length > FILE_CTRL_BUF_SIZE)
  {
    fileCtrlFlushForce(peri_name);

    if (p_file->write_length + length > FILE_CTRL_BUF_SIZE)
    {
      logPrintf("[ERR] fileCtrlWrite no space: %s, len:%d\n", p_file->file_path, length);
      return;
    }
  }

  // * 쓸 데이터를 버퍼에 계속 밀어넣어주다가
  memcpy(&p_file->write_buf[p_file->write_length], p_data, length);
  p_file->write_length += length;

  // * threshold만큼의 데이터가 있다고 판단했을때
  if (p_file->write_length >= FILE_CTRL_FLUSH_THRESHOLD)
  {
    // * 파일에 쓴다.
    fileCtrlFlush(peri_name);
  }
}

void fileCtrlFlush(PeriName_t peri_name)
{
  if (peri_name >= PERI_MAX)
    return;

  file_tbl_t *p_file = &file_tbl[peri_name];

  if (!p_file->is_open || p_file->write_length == 0)
    return;
  
  // *현재 버퍼에 쌓인 길이에서 FILE_CTRL_FLUSH_THRESHOLD바이트 단위로 딱 떨어지는 부분만 계산
  uint16_t write_size = (p_file->write_length / FILE_CTRL_FLUSH_THRESHOLD) * FILE_CTRL_FLUSH_THRESHOLD;

  if (write_size > 0)
  {
    // *써야할 데이터가 있다면 로깅을 시작한다는 로그와 함께 fatfs에 파일 기록 요청을 실시한다.
    if (fatfsWrite(&p_file->file_handler, (uint8_t *)p_file->write_buf, write_size))
    {
      if (!is_log_start_notified[peri_name])
      {
        logPrintf("[LOG] CAN logging started: %s\n", p_file->file_path);

        is_log_start_notified[peri_name] = true;
      }
    }
    else
    {
      logPrintf("[ERR] fileCtrl flush failed: %s\n", p_file->file_path);
    }
    
    // *자투리 데이터를 버퍼의 앞쪽으로 당겨준다.
    uint16_t remain = p_file->write_length - write_size;
    if (remain > 0)
    {
      memmove(p_file->write_buf, p_file->write_buf + write_size, remain);
    }

    // *버퍼내 데이터 길이 갱신
    p_file->write_length = remain;
  }
}

/**
 * @brief 모든 채널의 버퍼내 잔존 데이터를 강제로 쓴다.
 */
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

/**
 * @brief fatfs에게 단순히 쓰기 요청을 하는게 아닌 직접 쓴다.
 */
static void fileCtrlFlushForce(PeriName_t peri_name)
{
  if (peri_name >= PERI_MAX)
    return;

  file_tbl_t *p_file = &file_tbl[peri_name];

  if (!p_file->is_open || p_file->write_length == 0)
    return;

  if (fatfsWrite(&p_file->file_handler, (uint8_t *)p_file->write_buf, p_file->write_length))
  {
    if (!is_log_start_notified[peri_name])
    {
      logPrintf("[LOG] CAN logging started: %s\n", p_file->file_path);

      is_log_start_notified[peri_name] = true;
    }

    fatfsSync(&p_file->file_handler);
    p_file->write_length = 0;
  }
  else
  {
    logPrintf("[ERR] fileCtrl force flush failed: %s\n", p_file->file_path);
  }
}
