#include "file_controller.h"
#include "fatfs.h"

static void fileCtrlFlushForce(PeriName_t peri_name);

typedef struct
{
  FIL               file_handler;
  bool              is_open;
  char              write_buf[FILE_CTRL_BUF_SIZE];
  uint16_t          write_length;
  const char       *file_path;
} file_tbl_t;

__attribute__((section(".non_cache"))) static file_tbl_t file_tbl[PERI_MAX];

/**
 * @brief 파일이 없다면 파일을 새로 만들고, 파일을 여는등의 기본적인 file_open함수
 * @param name : 어떤 통신으로 받은 데이터인지?
 *        file_path : 해당 데이터는 어떤 경로에 기록되어야 하는지?
 *        csv_header : 엑셀 파일 최 상단 첫줄의 데이터의 종류에 대한 헤더
 */
bool fileCtrlOpen(PeriName_t name, const char *file_path, const char *csv_header)
{
  if (name >= PERI_MAX || file_path == NULL)
    return false;

  file_tbl_t *p_file = &file_tbl[name];

  bool is_new_file = !fatfsExist(file_path);

  // 파일을 열어준다.
  if (!fatfsOpen(&p_file->file_handler, file_path, FA_WRITE | FA_OPEN_APPEND))
  {
    p_file->is_open = false;
    return false;
  }

  p_file->is_open      = true;
  p_file->write_length = 0;
  p_file->file_path    = file_path;

  // 새 파일이면 CSV 컬럼명을 첫 줄에 기록
  if (is_new_file && csv_header != NULL)
  {
    fatfsWrite(&p_file->file_handler, (uint8_t *)csv_header, strlen(csv_header));
    fatfsSync(&p_file->file_handler);
  }

  logPrintf("[OK] fileCtrl opened: %s\n", p_file->file_path);

  return true;
}

/**
 * @brief 파일을 닫는 함수, 현재 버퍼내에 있는 데이터를 SD카드에 기록하고 파일을 닫는다.
 */
void fileCtrlClose(PeriName_t peri_name)
{
  if (peri_name >= PERI_MAX)
    return;

  file_tbl_t *p_file = &file_tbl[peri_name];

  if (!p_file->is_open)
    return;

  // 1). 파일을 닫기 전에는 남은 데이터가 1바이트라도 있으면 전부 기록한다.
  if (p_file->write_length > 0)
  {
    fileCtrlFlushForce(peri_name);
  }

  fatfsClose(&p_file->file_handler);
  p_file->is_open = false;

  logPrintf("[OK] fileCtrl closed: %s\n", p_file->file_path);
}


/**
 * @brief 입력 데이터를 버퍼에 저장하고, 버퍼가 차면 자동으로 SD카드에 기록
 */
void fileCtrlWrite(PeriName_t peri_name, const char *p_data, uint16_t length)
{
  if (peri_name >= PERI_MAX || p_data == NULL || length == 0)
    return;

  file_tbl_t *p_file = &file_tbl[peri_name];

  if (!p_file->is_open)
    return;

  // 1). 애초에 입력 데이터 한 줄이 버퍼보다 크면 기록할 수 없으니 막는다.
  if (length > FILE_CTRL_BUF_SIZE)
  {
    logPrintf("[ERR] fileCtrlWrite too large: %s, len:%d\n", p_file->file_path, length);
    return;
  }

  // 2). 이번 데이터를 더하면 buffer overflow가 예상될때 일단 일반 flush를 시도한다.
  if (p_file->write_length + length > FILE_CTRL_BUF_SIZE)
  {
    fileCtrlFlush(peri_name);

    // 3). 일반 flush는 512바이트 배수만 쓰므로, 아직도 공간이 부족하면 남은 것까지 전부 쓴다.
    if (p_file->write_length + length > FILE_CTRL_BUF_SIZE)
    {
      fileCtrlFlushForce(peri_name);
    }

    // 4). 강제 flush 뒤에도 공간이 안 나오면 더 이상 진행하지 않는다.
    if (p_file->write_length + length > FILE_CTRL_BUF_SIZE)
    {
      logPrintf("[ERR] fileCtrlWrite no space: %s, len:%d\n", p_file->file_path, length);
      return;
    }
  }

  // 5). 안전하게 들어갈 수 있을 때만 버퍼에 복사한다.
  memcpy(&p_file->write_buf[p_file->write_length], p_data, length);
  p_file->write_length += length;

  // 6). 평소에는 threshold 이상일 때만 일반 flush를 태워 성능을 챙긴다.
  if (p_file->write_length >= FILE_CTRL_FLUSH_THRESHOLD)
  {
    fileCtrlFlush(peri_name);
  }
}

/**
 * @brief  지정된 채널의 버퍼를 SD카드에 기록하고 버퍼를 비운다
 */
void fileCtrlFlush(PeriName_t peri_name)
{
  if (peri_name >= PERI_MAX)
    return;

  file_tbl_t *p_file = &file_tbl[peri_name];

  if (!p_file->is_open || p_file->write_length == 0)
    return;

  // 512 배수만큼만 쓰기
  uint16_t write_size = (p_file->write_length / 512) * 512;

  if (write_size > 0)
  {
    if (fatfsWrite(&p_file->file_handler, 
                   (uint8_t *)p_file->write_buf, write_size))
    {
      // Sync is deferred to force-flush/close so the writer thread can keep
      // draining queues even under sustained RX traffic.
    }
    else
    {
      logPrintf("[ERR] fileCtrl flush failed: %s\n", p_file->file_path);
    }

    // 남은 데이터를 버퍼 앞으로 이동
    uint16_t remain = p_file->write_length - write_size;
    if (remain > 0)
    {
      memmove(p_file->write_buf, 
              p_file->write_buf + write_size, remain);
    }
    p_file->write_length = remain;
  }
}

/**
 * @brief  모든 채널의 버퍼를 flush한다
 */
void fileCtrlFlushAll(void)
{
  for (int i = 0; i < PERI_MAX; i++)
  {
    // 1). 전체 flush는 종료/정리 성격이 강하므로 남은 버퍼를 전부 기록한다.
    if (file_tbl[i].write_length > 0)
    {
      fileCtrlFlushForce((PeriName_t)i);
    }
  }
}

/**
 * @brief 일반 flush는 정해진 threshold를 넘지 못하면 잉여 데이터를 저장할수없다.
 *        해당 함수는 찌꺼기 처럼 남은 잉여 데이터를 파일에 저장하는 함수이다.
 */
static void fileCtrlFlushForce(PeriName_t peri_name)
{
  if (peri_name >= PERI_MAX)
    return;

  file_tbl_t *p_file = &file_tbl[peri_name];

  // 1). 파일이 안 열려 있거나 쓸 데이터가 없으면 바로 종료한다.
  if (!p_file->is_open || p_file->write_length == 0)
    return;

  // 2). close 직전이나 공간 부족 상황에서는 남은 데이터를 전부 기록한다.
  if (fatfsWrite(&p_file->file_handler,
                 (uint8_t *)p_file->write_buf,
                 p_file->write_length))
  {
    fatfsSync(&p_file->file_handler);
    p_file->write_length = 0;
  }
  else
  {
    logPrintf("[ERR] fileCtrl force flush failed: %s\n", p_file->file_path);
  }
}


/**
 * @brief  지정된 채널의 파일이 현재 열려 있는지 확인
 */
bool fileCtrlIsOpen(PeriName_t peri_name)
{
  if (peri_name >= PERI_MAX)
    return false;

  return file_tbl[peri_name].is_open;
}
