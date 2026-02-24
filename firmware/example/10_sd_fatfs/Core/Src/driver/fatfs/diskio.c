/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2025        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* Declarations FatFs MAI */

// 아래 주석
// #include "platform.h"
// #include "storage.h"

#include "sdmmc.h" // sd객체 extern을 위해

#define DEV_FLASH	0	
// #define DEV_MMC		1	
// #define DEV_USB		2	

// SD 카드 작업 타임아웃
#define SD_TIMEOUT 1000  

// sd 핸들러
extern SD_HandleTypeDef hsd1;

// SD 카드 상태 플래그
static volatile DSTATUS status = STA_NOINIT;  // 초기 상태는 "초기화 안됨"

/**
 * @brief 디스크 상태 조회
 * 
 * FatFS가 디스크의 현재 상태를 확인할 때 호출합니다.
 * SD 카드가 초기화되었는지, 쓰기 보호되었는지 등을 반환합니다.
 * 
 * @param physical_driver_number 물리 드라이브 번호 (0: SD 카드, 1: USB 등)
 * @return DSTATUS 디스크 상태 플래그
 *         - 0: 정상
 *         - STA_NOINIT:  초기화 안됨
 *         - STA_NODISK:  디스크 없음
 *         - STA_PROTECT: 쓰기 보호됨
 */
DSTATUS disk_status(BYTE physical_driver_number)
{
  // 드라이브 번호 확인 (0번만 지원: SD 카드)
  if (physical_driver_number != DEV_FLASH)
  {
    return STA_NOINIT;  // 지원하지 않는 드라이브
  }

  // SD 카드의 현재 상태 반환
  return status;
}

/**
 * @brief 디스크 초기화
 * 
 * FatFS가 f_mount()를 호출할 때 내부적으로 이 함수를 실행합니다.
 * SD 카드를 사용 가능한 상태로 초기화합니다.
 * 
 * @param physical_driver_number 물리 드라이브 번호
 * @return DSTATUS 초기화 후 디스크 상태
 */
DSTATUS disk_initialize(BYTE physical_driver_number)
{
  // 드라이브 번호 확인
  if (physical_driver_number != DEV_FLASH)
  {
    return STA_NOINIT;
  }

  // SD 카드 초기화 상태 확인
  // HAL_SD_GetState(): SD 카드의 현재 상태를 반환
  // HAL_SD_STATE_READY: SD 카드가 정상적으로 초기화되어 사용 가능한 상태
  if (HAL_SD_GetState(&hsd1) == HAL_SD_CARD_READY)
  {
    status &= ~STA_NOINIT;  // 초기화 완료 플래그 설정 (비트 클리어)
  }
  else
  {
    status = STA_NOINIT;  // 초기화 실패
  }

  return status;
}

/**
 * @brief 섹터 읽기
 * 
 * SD 카드에서 지정된 섹터(블록)의 데이터를 읽습니다.
 * FatFS가 파일을 읽을 때 이 함수를 호출합니다.
 * 
 * @param physical_driver_number 물리 드라이브 번호
 * @param read_buffer            읽은 데이터를 저장할 버퍼
 * @param sector_start_num       읽기 시작할 섹터 번호 (LBA - Logical Block Address)
 * @param sector_count           읽을 섹터 개수
 * @return DRESULT 
 *         - RES_OK: 성공
 *         - RES_PARERR: 파라미터 오류
 *         - RES_NOTRDY: 디스크 준비 안됨
 *         - RES_ERROR: 읽기 실패
 */
DRESULT disk_read(BYTE physical_driver_number, BYTE *read_buffer, LBA_t sector_start_num, UINT sector_count)
{
  HAL_StatusTypeDef status;

  // 드라이브 번호 확인
  if (physical_driver_number != DEV_FLASH)
  {
    return RES_PARERR;  // 잘못된 파라미터
  }

  // 디스크가 초기화되었는지 확인
  if (status & STA_NOINIT)
  {
    return RES_NOTRDY;  // 디스크 준비 안됨
  }

  // 버퍼 주소가 NULL인지 확인
  if (read_buffer == NULL)
  {
    return RES_PARERR;
  }

  // SD 카드에서 데이터 읽기
  // sector_start_num: 읽기 시작 위치 (섹터 번호)
  // read_buffer:      읽은 데이터를 저장할 버퍼
  // sector_count:     읽을 섹터 개수 (1섹터 = 512바이트)
  status = HAL_SD_ReadBlocks(&hsd1, read_buffer, sector_start_num, sector_count, SD_TIMEOUT);

  // 읽기 결과 확인
  if (status != HAL_OK)
  {
    return RES_ERROR;  // 읽기 실패
  }

  // SD 카드가 읽기 완료될 때까지 대기
  // DMA를 사용하는 경우 이 과정이 중요합니다
  // 폴링 방식으로 SD 카드가 준비 상태가 될 때까지 기다림
  while (HAL_SD_GetCardState(&hsd1) != HAL_SD_CARD_TRANSFER)
  {

  }

  return RES_OK;  // 성공
}
/**
 * @brief 섹터 쓰기
 * 
 * SD 카드의 지정된 섹터에 데이터를 씁니다.
 * FatFS가 파일을 쓸 때 이 함수를 호출합니다.
 * 
 * @param physical_driver_number 물리 드라이브 번호
 * @param write_buffer 쓸 데이터가 담긴 버퍼
 * @param sector_start_num 쓰기 시작할 섹터 번호
 * @param sector_count 쓸 섹터 개수
 * @return DRESULT 쓰기 결과
 */
DRESULT disk_write(BYTE physical_driver_number, const BYTE *write_buffer, LBA_t sector_start_num, UINT sector_count)
{
  // 드라이브 번호 확인
  if (physical_driver_number != 0)
  {
    return RES_PARERR;
  }

  // 디스크가 초기화되었는지 확인
  if (status & STA_NOINIT)
  {
    return RES_NOTRDY;
  }

  // 쓰기 보호 확인
  if (status & STA_PROTECT)
  {
    return RES_WRPRT;  // 쓰기 보호됨
  }

  // 버퍼 주소가 NULL인지 확인
  if (write_buffer == NULL)
  {
    return RES_PARERR;
  }

  // SD 카드에 데이터 쓰기
  // sector_start_num: 쓰기 시작 위치 (섹터 번호)
  // write_buffer:     쓸 데이터가 담긴 버퍼
  // sector_count:     쓸 섹터 개수
  HAL_StatusTypeDef hal_result = HAL_SD_WriteBlocks(&hsd1, (uint8_t *)write_buffer, sector_start_num, sector_count, SD_TIMEOUT);

  // 쓰기 결과 확인
  if (hal_result != HAL_OK)
  {
    return RES_ERROR;
  }

  // SD 카드가 쓰기 완료될 때까지 대기
  // 이 단계에서 데이터가 실제로 SD 카드에 기록됨
  while (HAL_SD_GetCardState(&hsd1) != HAL_SD_CARD_TRANSFER)
  {
  }

  return RES_OK;
}

/**
 * @brief 디스크 입출력 제어
 * 
 * FatFS가 디스크의 물리적 특성을 조회하거나 특수 제어를 할 때 호출합니다.
 * 섹터 개수, 섹터 크기, 동기화 등의 명령을 처리합니다.
 * 
 * @param physical_driver_number 물리 드라이브 번호
 * @param cmd 제어 명령 코드
 *            - CTRL_SYNC: 쓰기 캐시 동기화
 *            - GET_SECTOR_COUNT: 전체 섹터 개수 조회
 *            - GET_SECTOR_SIZE: 섹터 크기 조회
 *            - GET_BLOCK_SIZE: 소거 블록 크기 조회
 * @param io_buff 명령에 따라 결과를 저장하거나 파라미터를 전달할 버퍼
 * @return DRESULT 제어 결과
 */
DRESULT disk_ioctl(BYTE physical_driver_number, BYTE cmd, void *io_buff)
{
  DRESULT res = RES_ERROR;
  HAL_SD_CardInfoTypeDef card_info;

  // 드라이브 번호 확인
  if (physical_driver_number != DEV_FLASH)
  {
    return RES_PARERR;
  }

  // 디스크가 초기화되었는지 확인
  if (status & STA_NOINIT)
  {
    return RES_NOTRDY;
  }

  // 명령 코드에 따라 분기 처리
  switch (cmd)
  {
    case CTRL_SYNC:
      // 쓰기 캐시 동기화 명령
      // SD 카드가 전송 준비 상태가 될 때까지 대기
      // 모든 쓰기 작업이 완료되었는지 확인
      while (HAL_SD_GetCardState(&hsd1) != HAL_SD_CARD_TRANSFER)
      {
        // 대기
      }
      res = RES_OK;
      break;

    case GET_SECTOR_COUNT:
      // 전체 섹터(블록) 개수 조회
      // SD 카드 정보 가져오기
      if (HAL_SD_GetCardInfo(&hsd1, &card_info) == HAL_OK)
      {
        // buff를 LBA_t(일반적으로 uint32_t) 포인터로 캐스팅하여
        // 전체 블록 개수를 저장
        // BlockNbr: SD 카드의 총 섹터 개수
        *(LBA_t *)io_buff = card_info.BlockNbr;
        res = RES_OK;
      }
      break;

    case GET_SECTOR_SIZE:
      // 섹터 크기 조회
      // SD 카드 정보 가져오기
      if (HAL_SD_GetCardInfo(&hsd1, &card_info) == HAL_OK)
      {
        // buff를 WORD(uint16_t) 포인터로 캐스팅하여
        // 한 섹터의 크기를 저장
        // BlockSize: 일반적으로 512바이트
        *(WORD *)io_buff = card_info.BlockSize;
        res = RES_OK;
      }
      break;

    case GET_BLOCK_SIZE:
      // 소거 블록 크기 조회
      // 한 번에 소거할 수 있는 최소 단위를 섹터 개수로 반환
      // 1을 반환: 1섹터 단위로 소거 가능
      // 이 값은 FatFS가 파일 시스템 포맷 시 
      // 클러스터 정렬을 최적화하는데 사용됨
      *(DWORD *)io_buff = 1;  // 1 sector
      res = RES_OK;
      break;

    case CTRL_TRIM:
      /* TRIM 명령 (옵션) */
      // 사용하지 않는 섹터를 SD 카드에 알려주는 명령
      // 성능 최적화에 도움이 되지만 필수는 아님
      res = RES_OK;
      break;

    default:
      // 지원하지 않는 명령
      res = RES_PARERR;
      break;
  }

  return res;
}

/**
 * @brief 현재 시간 가져오기 (타임스탬프용)
 * 
 * FatFS가 파일의 생성/수정 시간을 기록할 때 이 함수를 호출합니다.
 * RTC(Real-Time Clock)가 있으면 실제 시간을 반환하고,
 * 없으면 고정된 시간을 반환합니다.
 * 
 * @return DWORD FAT 파일 시스템 타임스탬프 형식
 *         - bit 31:25 - 년도 (0 = 1980년부터 시작)
 *         - bit 24:21 - 월 (1~12)
 *         - bit 20:16 - 일 (1~31)
 *         - bit 15:11 - 시 (0~23)
 *         - bit 10:5  - 분 (0~59)
 *         - bit 4:0   - 초/2 (0~29, 실제 초는 × 2)
 */
DWORD get_fattime(void)
{
//   // RTC가 있는 경우 실제 시간을 사용하는 예시 
//   RTC_TimeTypeDef sTime;
//   RTC_DateTypeDef sDate;
  
//   HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
//   HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
  
//   return ((DWORD)(sDate.Year + 20) << 25)  // 2000 + Year
//        | ((DWORD)sDate.Month << 21)
//        | ((DWORD)sDate.Date << 16)
//        | ((DWORD)sTime.Hours << 11)
//        | ((DWORD)sTime.Minutes << 5)
//        | ((DWORD)sTime.Seconds >> 1);
  

  // RTC가 없는 경우 고정 시간 반환 
  // 예: 2024년 1월 1일 00:00:00
  return ((DWORD)(2024 - 1980) << 25)  // 년도: 2024
       | ((DWORD)1 << 21)              // 월: 1월
       | ((DWORD)1 << 16)              // 일: 1일
       | ((DWORD)0 << 11)              // 시: 0시
       | ((DWORD)0 << 5)               // 분: 0분
       | ((DWORD)0 >> 1);              // 초: 0초
}
