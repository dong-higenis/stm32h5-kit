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
  
  // *로그파일 5MB제한 
  uint32_t file_size;
  uint16_t file_index;
} file_tbl_t;

static void fileCtrlFlushForce(PeriName_t peri_name);

__attribute__((section(".non_cache"))) static file_tbl_t file_tbl[PERI_MAX];
static bool        is_log_start_notified[PERI_MAX] = {false,};
static const char *file_base_path[PERI_MAX]        = {NULL,};
static const char *file_csv_header[PERI_MAX]       = {NULL,};
static char        file_path_buf[PERI_MAX][FILE_CTRL_MAX_PATH_LEN];

bool fileCtrlOpen(PeriName_t name, const char *file_path, const char *csv_header)
{
  char  dir_path[FILE_CTRL_MAX_PATH_LEN];
  char *p_last_slash;

  // * 유효하지 않은 채널 번호이거나 파일 경로가 없으면 열기를 종료한다.
  if (name >= PERI_MAX || file_path == NULL)
    return false;

  file_base_path[name]  = file_path;
  file_csv_header[name] = csv_header;

  // * 채널 폴더가 아직 없다면 먼저 생성해 준다.
  memset(dir_path, 0, sizeof(dir_path));
  snprintf(dir_path, sizeof(dir_path), "%s", file_path);

  p_last_slash = strrchr(dir_path, '/');
  if (p_last_slash == NULL)
    return false;

  *p_last_slash = '\0';

  if (!fatfsExist(dir_path))
  {
    if (!fatfsCreateDir(dir_path))
      return false;
  }

  // * 파일 번호를 00부터 순서대로 확인해 비어 있는 첫 번째 파일을 찾는다.
  for (file_tbl[name].file_index = 0; file_tbl[name].file_index < FILE_CTRL_MAX_FILE_INDEX; file_tbl[name].file_index++)
  {
    if (snprintf(file_path_buf[name], sizeof(file_path_buf[name]), "%s_%02u.csv", file_path, file_tbl[name].file_index) >= sizeof(file_path_buf[name]))
      return false;

    if (!fatfsExist(file_path_buf[name]))
      break;
  }

  if (file_tbl[name].file_index >= FILE_CTRL_MAX_FILE_INDEX)
    return false;

  // * 찾은 번호의 파일을 새로 생성해서 연다.
  if (!fatfsOpen(&file_tbl[name].file_handler, file_path_buf[name], FA_WRITE | FA_CREATE_NEW))
  {
    file_tbl[name].is_open = false;
    return false;
  }

  // * open 성공 후 내부 상태를 초기화한다.
  file_tbl[name].is_open      = true;
  file_tbl[name].write_length = 0;
  file_tbl[name].file_path    = file_path_buf[name];
  file_tbl[name].file_size    = 0;

  // * 새 파일에만 CSV 첫 줄 헤더를 기록하고 바로 sync 한다.
  if (csv_header != NULL)
  {
    uint32_t header_len = strlen(csv_header);

    if (!fatfsWrite(&file_tbl[name].file_handler, (uint8_t *)csv_header, header_len))
    {
      fatfsClose(&file_tbl[name].file_handler);
      file_tbl[name].is_open = false;
      logPrintf("[ERR] fileCtrl header write failed: %s\n", file_tbl[name].file_path);
      return false;
    }

    fatfsSync(&file_tbl[name].file_handler);
    file_tbl[name].file_size = header_len;
  }

  is_log_start_notified[name] = false;
  logPrintf("[OK] fileCtrl opened: %s\n", file_tbl[name].file_path);

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

  // * 써야 할 데이터가 있다면 모두 쓴다.
  if (p_file->write_length > 0)
  {
    fileCtrlFlushForce(peri_name);
  }

  // * 파일을 닫아준다.
  fatfsClose(&p_file->file_handler);
  p_file->is_open = false;

  logPrintf("[OK] fileCtrl closed: %s\n", p_file->file_path);
}

bool fileCtrlWrite(PeriName_t peri_name, const char *p_data, uint16_t length)
{
  file_tbl_t *p_file;

  if (peri_name >= PERI_MAX || p_data == NULL || length == 0)
    return false;

  p_file = &file_tbl[peri_name];

  // * 파일이 열려있는지 그 여부를 파악한다.
  if (!p_file->is_open)
    return false;

  // * 만약 새 데이터를 버퍼에 넣기 전에, 공간이 부족하면 먼저 한 번 flush해서 자리를 만들어 준다.
  if (p_file->write_length + length > FILE_CTRL_BUF_SIZE)
  {
    fileCtrlFlushForce(peri_name);

    if (p_file->write_length + length > FILE_CTRL_BUF_SIZE)
    {
      logPrintf("[ERR] fileCtrlWrite no space: %s, len:%d\n", p_file->file_path, length);
      return false;
    }
  }

  // * 현재 파일 크기와 버퍼에 쌓인 데이터까지 합쳐서 5MB를 넘기면 다음 파일로 넘긴다.
  if (p_file->file_size + p_file->write_length + length > FILE_CTRL_MAX_FILE_SIZE)
  {
    fileCtrlClose(peri_name);

    if (file_base_path[peri_name] == NULL)
      return false;

    p_file->file_index++;

    while (p_file->file_index < FILE_CTRL_MAX_FILE_INDEX)
    {
      if (snprintf(file_path_buf[peri_name], sizeof(file_path_buf[peri_name]), "%s_%02u.csv", file_base_path[peri_name], p_file->file_index) >= sizeof(file_path_buf[peri_name]))
        return false;

      if (!fatfsExist(file_path_buf[peri_name]))
        break;

      p_file->file_index++;
    }

    if (p_file->file_index >= FILE_CTRL_MAX_FILE_INDEX)
      return false;

    if (!fatfsOpen(&p_file->file_handler, file_path_buf[peri_name], FA_WRITE | FA_CREATE_NEW))
    {
      p_file->is_open = false;
      return false;
    }

    p_file->is_open      = true;
    p_file->write_length = 0;
    p_file->file_path    = file_path_buf[peri_name];
    p_file->file_size    = 0;

    if (file_csv_header[peri_name] != NULL)
    {
      uint32_t header_len = strlen(file_csv_header[peri_name]);

      if (!fatfsWrite(&p_file->file_handler, (uint8_t *)file_csv_header[peri_name], header_len))
      {
        fatfsClose(&p_file->file_handler);
        p_file->is_open = false;
        logPrintf("[ERR] fileCtrl header write failed: %s\n", p_file->file_path);
        return false;
      }

      fatfsSync(&p_file->file_handler);
      p_file->file_size = header_len;
    }

    is_log_start_notified[peri_name] = false;
    logPrintf("[OK] fileCtrl opened: %s\n", p_file->file_path);
  }

  // * 새 데이터를 버퍼에 계속 쌓아넣어준다.
  memcpy(&p_file->write_buf[p_file->write_length], p_data, length);
  p_file->write_length += length;

  // * threshold만큼의 데이터가 있다고 판단되었을때
  if (p_file->write_length >= FILE_CTRL_FLUSH_THRESHOLD)
  {
    // * 파일에 쓴다.
    fileCtrlFlush(peri_name);
  }

  return true;
}

void fileCtrlFlush(PeriName_t peri_name)
{
  uint16_t write_size;
  uint16_t remain;

  if (peri_name >= PERI_MAX)
    return;

  if (!file_tbl[peri_name].is_open || file_tbl[peri_name].write_length == 0)
    return;
  
  // *현재 버퍼에 쌓인 길이에서 FILE_CTRL_FLUSH_THRESHOLD바이트 단위로 딱 떨어지는 부분만 계산
  write_size = (file_tbl[peri_name].write_length / FILE_CTRL_FLUSH_THRESHOLD) * FILE_CTRL_FLUSH_THRESHOLD;

  if (write_size > 0)
  {
    // *써야할 데이터가 있다면 로깅을 시작한다는 로그와 함께 실제 fatfs에 파일 기록 요청을 지시한다.
    if (fatfsWrite(&file_tbl[peri_name].file_handler, (uint8_t *)file_tbl[peri_name].write_buf, write_size))
    {
      file_tbl[peri_name].file_size += write_size;

      if (!is_log_start_notified[peri_name])
      {
        logPrintf("[LOG] CAN logging started: %s\n", file_tbl[peri_name].file_path);

        is_log_start_notified[peri_name] = true;
      }
    }
    else
    {
      logPrintf("[ERR] fileCtrl flush failed: %s\n", file_tbl[peri_name].file_path);
    }
    
    // *자투리 데이터를 버퍼의 앞쪽으로 당겨준다.
    remain = file_tbl[peri_name].write_length - write_size;
    if (remain > 0)
    {
      memmove(file_tbl[peri_name].write_buf, file_tbl[peri_name].write_buf + write_size, remain);
    }

    // *버퍼의 데이터 길이 갱신
    file_tbl[peri_name].write_length = remain;
  }
}

/**
 * @brief 모든 채널의 버퍼에 남은 데이터를 강제로 쓴다.
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
 * @brief fatfs에게 단순 쓰기 요청을 하는게 아닌 직접 쓴다.
 */
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
    file_tbl[peri_name].file_size += write_size;

    if (!is_log_start_notified[peri_name])
    {
      logPrintf("[LOG] CAN logging started: %s\n", file_tbl[peri_name].file_path);

      is_log_start_notified[peri_name] = true;
    }

    fatfsSync(&file_tbl[peri_name].file_handler);
    file_tbl[peri_name].write_length = 0;
  }
  else
  {
    logPrintf("[ERR] fileCtrl force flush failed: %s\n", file_tbl[peri_name].file_path);
  }
}
