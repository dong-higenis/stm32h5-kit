#include "custom_sd.h"

// fatfs를 사용하기 위해 아래 두 파일을 include
#include "diskio.h"
#include "ff.h"

#include "sdmmc.h" // sd객체 extern을 위해

#include <string.h>
#include <stdio.h>
/**
 * @brief SD 카드 핸들
 */
extern SD_HandleTypeDef hsd1; // sd 핸들러 참조
static FATFS            SDFatFs;
static bool             is_init        = false;
static bool             fatfs_mounted  = false;

/**
 * @brief 내부 함수 선언
 */
static bool customSdMount(void);
static bool customSdUnmount(void);
static bool customSdWriteFile(const char *filename, const char *data);
static bool customSdReadFile(const char *filename);

/**
 * @brief SD 카드 초기화
 */
bool customSdInit(void)
{
  if (is_init)
  {
    return true;
  }
  
  // .ioc에서 설정한 값을 여기서 직접합니다.
  hsd1.Instance = SDMMC1;
  hsd1.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
  hsd1.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  hsd1.Init.BusWide = SDMMC_BUS_WIDE_4B;
  hsd1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_ENABLE;
  hsd1.Init.ClockDiv = 4;

  if (HAL_SD_Init(&hsd1) != HAL_OK)
  {
    return false;
  }

  is_init = true;
  return true;
}

/**
 * @brief FatFs를 이용한 SD카드 마운트
 */
bool customSdMount(void)
{
  if (fatfs_mounted) // 이미 마운트 되었다면 true 리턴
  {
    return true;
  }

  if (!is_init)
  {
    if (!customSdInit()) // 초기화가 되어있지 않다면 초기화 시도
    {
      return false;
    }
  }

  FRESULT result = f_mount(&SDFatFs, "0:", 1); // 절대 경로로 마운트 시도, 마지막 인자는 옵션이며 '1'값은 즉시 마운트

  if (result != FR_OK)
  {
    return false;
  }

  fatfs_mounted = true;
  return true;
}

/**
 * @brief FatFs 언마운트
 */
bool customSdUnmount(void)
{
  if (!fatfs_mounted)
  {
    return true;
  }

  FRESULT result = f_mount(NULL, "0:", 1); // 파일 시스템을 빼면, unmount

  if (result != FR_OK)
  {
    return false;
  }

  fatfs_mounted = false;
  return true;
}

/**
 * @brief SD 카드에 파일 쓰기 함수
 * 
 * 이 함수는 지정된 파일에 데이터를 추가(append) 방식으로 씁니다.
 * 파일이 존재하지 않으면 새로 생성하고, 존재하면 파일 끝에 데이터를 추가합니다.
 * 
 * @param filename 쓰기를 수행할 파일의 이름 (예: "log.txt")
 * @param data 파일에 쓸 문자열 데이터 (null 종료 문자열)
 * @return true 쓰기 성공, false 쓰기 실패
 */
bool customSdWriteFile(const char *filename, const char *data)
{
  if (!fatfs_mounted)
  {
    if (!customSdMount())
    {
      return false;
    }
  }

  FIL     file;            // FatFS 파일 객체
  FRESULT fatfs_result;    // FatFS 함수 실행 결과를 저장할 변수
  UINT    bytes_written;   // 실제로 쓰여진 바이트 수를 저장할 변수

  char path[64];

  // "0:" 접두사를 붙여 전체 경로 생성 (0번 드라이브를 의미)
  snprintf(path, sizeof(path), "0:%s", filename);

  // 파일 열기
  // FA_OPEN_APPEND: 파일이 있으면 끝에 추가, 없으면 새로 생성
  // FA_WRITE: 쓰기 모드로 열기
  fatfs_result = f_open(&file, path, FA_OPEN_APPEND | FA_WRITE);

  if (fatfs_result != FR_OK)
  {
    return false;
  }

  // 데이터 쓰기 : 실제 sd카드 내에 쓰여지는게 아닌, 일시적으로 RAM에 기록 함
  fatfs_result = f_write(&file, data, strlen(data), &bytes_written); // bytes_written 인자를 통해 쓰여진 바이트 수를 반환한다.
  if (fatfs_result != FR_OK || bytes_written != strlen(data))        // 쓰기 실패 또는 요청한 크기만큼 쓰지 못한 경우
  {
    f_close(&file); // 파일을 닫고
    return false;   // 실패를 반환한다.
  }
  
  // 줄 바꿈 추가
  const char newline = '\n';
  fatfs_result = f_write(&file, &newline, 1, &bytes_written);

  // 버퍼에 있는 데이터를 실제 저장 매체에 기록 (동기화)
  // 전원이 갑자기 꺼져도 데이터 손실을 최소화
  // RAM에 기록된 데이터를 -> 실제 SD카드에 기록
  f_sync(&file);

  // 파일 닫기 (리소스 해제)
  f_close(&file);

  return true;
}

/**
 * @brief SD 카드에서 파일 읽기 및 콘솔 출력 함수
 * 
 * 이 함수는 지정된 파일의 내용을 읽어서 printf를 통해 출력합니다.
 * 파일을 128바이트 단위로 읽어 버퍼를 통해 순차적으로 출력합니다.
 * 
 * @param filename 읽을 파일의 이름 (예: "log.txt")
 * @return true 읽기 성공, false 읽기 실패
 */
bool customSdReadFile(const char *filename)
{
  if (!fatfs_mounted)
  {
    if (!customSdMount())
    {
      return false;
    }
  }

  FIL     file;            
  FRESULT fatfs_result;    
  UINT    bytes_readed;    // 실제로 읽은 바이트 수를 저장할 변수 (Bytes Read)
  char    buffer[128];     // 파일 내용을 임시로 저장할 버퍼 (128바이트)

  char path[64];
  snprintf(path, sizeof(path), "0:%s", filename);

  // 파일 열기 (읽기 전용)
  fatfs_result = f_open(&file, path, FA_READ);
  if (fatfs_result != FR_OK)
  {
    return false;
  }

  // 파일 내용 읽기
  // 파일을 끝까지 읽는 반복문
  // f_read가 성공하고(FR_OK) 읽은 바이트가 0보다 크면 계속 반복
  while ((fatfs_result = f_read(&file, buffer, sizeof(buffer) - 1, &bytes_readed)) == FR_OK && bytes_readed > 0)
  {
    buffer[bytes_readed] = '\n'; // null 종료
    cliPrintf("%s", buffer);
  }

  // 파일 닫기
  f_close(&file);

  return true;
}

/**
 * @brief CLI 명령어 처리
 */
void cliSdCard(cli_args_t *args)
{
  bool ret = false;

  if (args->argc == 1 && args->isStr(0, "info"))
  {
    cliPrintf("SD Init: %s\n", is_init ? "OK" : "FAIL");
    cliPrintf("FatFs Mounted: %s\n", fatfs_mounted ? "YES" : "NO");

    if (is_init)
    {
      // SD 카드의 상세 정보를 저장할 구조체 선언
      // HAL(Hardware Abstraction Layer) 라이브러리에서 제공하는 타입
      HAL_SD_CardInfoTypeDef card_info;

      // SD 카드 핸들(&hsd)로부터 카드 정보를 읽어와서 card_info에 저장
      // 블록 개수, 블록 크기, 카드 타입 등의 정보를 가져옴
      HAL_SD_GetCardInfo(&hsd1, &card_info);
     
      // SD 카드의 전체 용량을 MB 단위로 계산
      // 계산 과정:
      // 1. BlockNbr: 전체 블록 개수
      // 2. BlockSize: 한 블록의 크기(바이트)
      // 3. BlockNbr × BlockSize = 전체 용량(바이트)
      // 4. ÷ 1024 = KB 단위로 변환
      // 5. ÷ 1024 = MB 단위로 변환
      // uint64_t로 캐스팅하는 이유: 큰 용량의 SD 카드에서 오버플로우 방지
      uint32_t card_size_mb = (uint32_t)((uint64_t)card_info.BlockNbr * (uint64_t)card_info.BlockSize / 1024 / 1024);

      cliPrintf("Card Type: %d\n", card_info.CardType);
      cliPrintf("Card Size: %d MB\n", card_size_mb);
      cliPrintf("Block Count: %d\n", card_info.BlockNbr);
      cliPrintf("Block Size: %d bytes\n", card_info.BlockSize);
    }

    ret = true;
  }
  else if (args->argc == 1 && args->isStr(0, "mount"))
  {
    if (customSdMount())
    {
      cliPrintf("FatFs Mount OK\n");
    }
    else
    {
      cliPrintf("FatFs Mount FAIL\n");
    }
    ret = true;
  }
  else if (args->argc == 1 && args->isStr(0, "unmount"))
  {
    if (customSdUnmount())
    {
      cliPrintf("FatFs Unmount OK\n");
    }
    else
    {
      cliPrintf("FatFs Unmount FAIL\n");
    }
    ret = true;
  }
  else if (args->argc == 3 && args->isStr(0, "write"))
  {
    const char *filename = args->getStr(1);
    const char *data     = args->getStr(2);

    if (customSdWriteFile(filename, data))
    {
      cliPrintf("Write OK: %s\n", filename);
    }
    else
    {
      cliPrintf("Write FAIL\n");
    }
    ret = true;
  }
  else if (args->argc == 2 && args->isStr(0, "read"))
  {
    const char *filename = args->getStr(1);

    cliPrintf("--- File Content: %s ---\n", filename);
    if (customSdReadFile(filename))
    {
      cliPrintf("\n--- End ---\n");
    }
    else
    {
      cliPrintf("Read FAIL\n");
    }
    ret = true;
  }

  if (!ret)
  {
    cliPrintf("sd info              - SD 카드 정보 출력\n");
    cliPrintf("sd mount             - FatFs 마운트\n");
    cliPrintf("sd unmount           - FatFs 언마운트\n");
    cliPrintf("sd write [file] [data] - 파일에 데이터 쓰기\n");
    cliPrintf("sd read [file]       - 파일 읽기\n");
  }
}
