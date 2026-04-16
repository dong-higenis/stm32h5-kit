#include "file_controller.h"
#include "fatfs.h"

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
  {
    return;
  }

  if (p_file->write_length > 0)
  {
    fileCtrlFlush(peri_name);
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

  // 버퍼가 가득 찼다면 버퍼에 있는 데이터를 SD카드에 기록한다.
  if (p_file->write_length + length >= FILE_CTRL_BUF_SIZE)
  {
    fileCtrlFlush(peri_name);
  }

  // 버퍼내 기록할 위치에 데이터를 복사한다.
  memcpy(&p_file->write_buf[p_file->write_length], p_data, length);

  // 다음 기록을 위해 인덱스를 증가시킨다.
  p_file->write_length += length;

  // 가득은 아니더라도 어느정도 버퍼가 차있다면, 데이터를 SD카드에 기록한다.
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
      fatfsSync(&p_file->file_handler);
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
    if (file_tbl[i].write_length > 0)
    {
      fileCtrlFlush((PeriName_t)i);
    }
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

/**
 * @brief  SD카드가 마운트되어 있는지 확인 (래퍼 함수)
 */
bool fileCtrlCheckSD(void)
{
  return fatfsIsMount();
}