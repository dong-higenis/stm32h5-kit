#include "sd.h"



#ifdef _USE_HW_SD
#include "gpio.h"
#include "cli.h"




static bool is_init = false;
static bool is_detected = false;
static volatile bool is_rx_done = false;
static volatile bool is_tx_done = false;
static uint8_t is_try = 0;
static sd_state_t sd_state = SDCARD_IDLE;
static const uint8_t _PIN_GPIO_SDCARD_DETECT = (uint8_t)SD_CD;

extern SD_HandleTypeDef hsd1;

#ifdef _USE_HW_SD
static void cliSd(cli_args_t *args);
#endif

bool sdInit(void)
{
  bool ret = false;

  // 100Mhz / (1+1) = 50Mhz
  //
  hsd1.Instance            = SDMMC1;
  hsd1.Init.ClockEdge      = SDMMC_CLOCK_EDGE_RISING;
  hsd1.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  hsd1.Init.BusWide        = SDMMC_BUS_WIDE_4B;
  hsd1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_ENABLE;
  hsd1.Init.ClockDiv       = 1;

  is_detected = sdIsDetected();

  if (is_detected == true)
  {
    if (HAL_SD_Init(&hsd1) == HAL_OK)
    {
      ret = true;
    }
  }

  is_init = ret;


  static bool is_reinit = false;

  if (is_reinit == false)
  {
    logPrintf("[%s] sdInit()\n", ret ? "OK":"E_");
    if (is_detected == true)
    {
      uint32_t sdmmc_clk;

      sdmmc_clk = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SDMMC1) / (hsd1.Init.ClockDiv + 1);
      logPrintf("     sdcard found\n");
      logPrintf("     clk : %d MHz\n", sdmmc_clk/1000000);
    }
    else
    {
      logPrintf("     sdcard not found\n");
    }
  }


#ifdef _USE_HW_SD
  if (is_reinit == false)
    cliAdd("sd", cliSd);
#endif

  is_reinit = true;

  return ret;
}

bool sdReInit(void)
{
  bool ret = false;

  HAL_SD_DeInit(&hsd1);
  if (HAL_SD_Init(&hsd1) == HAL_OK)
  {
    ret = true;
  }

  is_init = ret;

  return ret;
}

bool sdDeInit(void)
{
  bool ret = false;

  if (is_init == true)
  {
    is_init = false;
    if (HAL_SD_DeInit(&hsd1) == HAL_OK)
    {
      ret = true;
    }
  }

  return ret;
}

bool sdIsInit(void)
{
  return is_init;
}

bool sdIsDetected(void)
{
  bool detected = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4) == false);

  static bool first_connected = false; // init failed 방지

  // 디바운싱을 위한 간단한 필터링 (선택사항)
  static uint32_t pre_time = 0;
  static bool prev_detected = false;

  if (detected != prev_detected)
  {
    if (millis() - pre_time > 50 || first_connected == false)  // 50ms 디바운스
    {
      first_connected = true;
      prev_detected = detected;
    }
  }
  else
  {
    pre_time = millis();
  }

  return prev_detected;
}

sd_state_t sdUpdate(void)
{
  sd_state_t ret_state = SDCARD_IDLE;
  static uint32_t pre_time;
  bool detected = sdIsDetected();
  static bool prev_detected = false;

  if (detected != prev_detected)
  {
    prev_detected = detected;
  }


  switch(sd_state)
  {
    case SDCARD_IDLE:
      if (detected == true)
      {
        if (is_init)
        {
          sd_state = SDCARD_CONNECTED;
        }
        else
        {
          sd_state = SDCARD_CONNECTTING;
          pre_time = millis();
          is_try = 0;
        }
      }
      else
      {
        if (is_init)
        {
          is_init = false;
          sd_state = SDCARD_DISCONNECTED;
          ret_state = SDCARD_DISCONNECTED;
          logPrintf("SD disconnected\n");
        }
      }
      break;

    case SDCARD_CONNECTTING:
      if (millis()-pre_time >= 100)
      {
        if (sdReInit())
        {
          sd_state = SDCARD_CONNECTED;
          ret_state = SDCARD_CONNECTED;
          logPrintf("SD connected\n");
          is_try = 0;
        }
        else
        {
          sd_state = SDCARD_IDLE;
          is_try++;

          if (is_try >= 3)
          {
            sd_state = SDCARD_ERROR;
            logPrintf("SD init failed after %d tries\n", is_try);
          }
        }
      }
      break;

    case SDCARD_CONNECTED:
      if (detected != true)  // ★ 최신 detected 사용
      {
        is_init = false;
        is_try = 0;
        sd_state = SDCARD_IDLE;
        ret_state = SDCARD_DISCONNECTED;
        logPrintf("SD removed\n");
      }
      break;

    case SDCARD_DISCONNECTED:
      if (detected == true)
      {
        is_try = 0;
        sd_state = SDCARD_IDLE;  // ★ 재연결 시도
        logPrintf("SD inserted, retrying...\n");  // ★ insert 감지 log
      }
      break;

    case SDCARD_ERROR:
      if (detected == true)
      {
        is_try = 0;
        sd_state = SDCARD_IDLE;  // ★ error 복구 시도
      }
      break;
  }

  is_detected = detected;  // ★ 전역 캐시 업데이트
  return ret_state;
}

bool sdGetInfo(sd_info_t *p_info)
{
  bool ret = false;
  sd_info_t *p_sd_info = (sd_info_t *)p_info;

  HAL_SD_CardInfoTypeDef card_info;


  if (is_init == true)
  {
    HAL_SD_GetCardInfo(&hsd1, &card_info);

    p_sd_info->card_type          = card_info.CardType;
    p_sd_info->card_version       = card_info.CardVersion;
    p_sd_info->card_class         = card_info.Class;
    p_sd_info->rel_card_Add       = card_info.RelCardAdd;
    p_sd_info->block_numbers      = card_info.BlockNbr;
    p_sd_info->block_size         = card_info.BlockSize;
    p_sd_info->log_block_numbers  = card_info.LogBlockNbr;
    p_sd_info->log_block_size     = card_info.LogBlockSize;
    p_sd_info->card_size          =  (uint32_t)((uint64_t)p_sd_info->block_numbers * (uint64_t)p_sd_info->block_size / (uint64_t)1024 / (uint64_t)1024);
    ret = true;
  }

  return ret;
}

bool sdIsBusy(void)
{
  bool is_busy;

  if (HAL_SD_GetCardState(&hsd1) == HAL_SD_CARD_TRANSFER )
  {
    is_busy = false;
  }
  else
  {
    is_busy = true;
  }

  return is_busy;
}

bool sdIsReady(uint32_t timeout)
{
  uint32_t pre_time;

  pre_time = millis();

  while(millis() - pre_time < timeout)
  {
    if (sdIsBusy() == false)
    {
      return true;
    }
  }

  return false;
}

bool sdReadBlocks(uint32_t block_addr, uint8_t *p_data, uint32_t num_of_blocks, uint32_t timeout_ms)
{
  bool ret = false;


  if (is_init == false) return false;

  #ifdef HW_SD_USE_DMA
  uint32_t pre_time;

  is_rx_done = false;
  if(HAL_SD_ReadBlocks_DMA(&hsd1, (uint8_t *)p_data, block_addr, num_of_blocks) == HAL_OK)
  {

    pre_time = millis();
    while(is_rx_done == false)
    {
      if (millis()-pre_time >= timeout_ms)
      {
        break;
      }
    }
    while(sdIsBusy() == true)
    {
      if (millis()-pre_time >= timeout_ms)
      {
        is_rx_done = false;
        break;
      }
    }
    ret = is_rx_done;
  }
  #else
  if(HAL_SD_ReadBlocks(&hsd1, (uint8_t *)p_data, block_addr, num_of_blocks, timeout_ms) == HAL_OK)
  {
    ret = true;
  }
  #endif

  if (ret == true)
  {
    #ifdef _USE_HW_CACHE
    SCB_InvalidateDCache_by_Addr((uint32_t*)p_data, BLOCKSIZE * num_of_blocks);
    #endif
  }
  return ret;
}

bool sdWriteBlocks(uint32_t block_addr, uint8_t *p_data, uint32_t num_of_blocks, uint32_t timeout_ms)
{
  bool ret = false;

  if (is_init == false) return false;


  #ifdef _USE_HW_CACHE
  SCB_InvalidateDCache_by_Addr((uint32_t *)p_data, num_of_blocks * BLOCKSIZE);
  #endif

  #ifdef HW_SD_USE_DMA
  uint32_t pre_time;

  is_tx_done = false;
  if(HAL_SD_WriteBlocks_DMA(&hsd1, (uint8_t *)p_data, block_addr, num_of_blocks) == HAL_OK)
  {
    pre_time = millis();
    while(is_tx_done == false)
    {
      if (millis()-pre_time >= timeout_ms)
      {
        break;
      }
    }
    pre_time = millis();
    while(sdIsBusy() == true)
    {
      if (millis()-pre_time >= timeout_ms)
      {
        is_tx_done = false;
        break;
      }
    }
    ret = is_tx_done;
  }
  #else
  if(HAL_SD_WriteBlocks(&hsd1, (uint8_t *)p_data, block_addr, num_of_blocks, timeout_ms) == HAL_OK)
  {
    ret = true;
  }
  #endif

  return ret;
}

bool sdEraseBlocks(uint32_t start_addr, uint32_t end_addr)
{
  bool ret = false;

  if (is_init == false) return false;

  if(HAL_SD_Erase(&hsd1, start_addr, end_addr) == HAL_OK)
  {
    ret = true;
  }

  return ret;
}

void HAL_SD_RxCpltCallback(SD_HandleTypeDef *hsd)
{
  (void)hsd;

  is_rx_done = true;
}

void HAL_SD_TxCpltCallback(SD_HandleTypeDef *hsd)
{
  (void)hsd;

  is_tx_done = true;
}

#ifdef _USE_HW_SD
void cliSd(cli_args_t *args)
{
  bool ret = false;


  if (args->argc == 1 && args->isStr(0, "info") == true)
  {
    sd_info_t sd_info;

    cliPrintf("sd init      : %d\n", is_init);
    cliPrintf("sd connected : %d\n", is_detected);

    if (is_init == true)
    {
      if (sdGetInfo(&sd_info) == true)
      {
        cliPrintf("   card_type            : %d\n", sd_info.card_type);
        cliPrintf("   card_version         : %d\n", sd_info.card_version);
        cliPrintf("   card_class           : %d\n", sd_info.card_class);
        cliPrintf("   rel_card_Add         : %d\n", sd_info.rel_card_Add);
        cliPrintf("   block_numbers        : %d\n", sd_info.block_numbers);
        cliPrintf("   block_size           : %d\n", sd_info.block_size);
        cliPrintf("   log_block_numbers    : %d\n", sd_info.log_block_numbers);
        cliPrintf("   log_block_size       : %d\n", sd_info.log_block_size);
        cliPrintf("   card_size            : %d MB, %d.%d GB\n", sd_info.card_size, sd_info.card_size/1024, ((sd_info.card_size * 10)/1024) % 10);
      }
    }
    ret = true;
  }

  if (args->argc == 2 && args->isStr(0, "read") == true)
  {
    uint32_t number;
    uint32_t buf[512/4];

    number = args->getData(1);

    if (sdReadBlocks(number, (uint8_t *)buf, 1, 100) == true)
    {
      for (int i=0; i<512/4; i++)
      {
        cliPrintf("%d:%04d : 0x%08X\n", number, i*4, buf[i]);
      }
    }
    else
    {
      cliPrintf("sdRead Fail\n");
    }

    ret = true;
  }

  if (args->argc == 1 && args->isStr(0, "speed-test") == true)
  {
    uint32_t number = 0;  // ← 0으로 초기화!
    uint32_t buf[512/4];
    uint32_t cnt;
    uint32_t pre_time;
    uint32_t exe_time;

    cnt = 1024*1024 / 512;
    pre_time = millis();
    for (int i=0; i<(int)cnt; i++)
    {
      if (sdReadBlocks(number, (uint8_t *)buf, 1, 100) == false)
      {
        cliPrintf("sdReadBlocks() Fail:%d\n", i);
        break;
      }
    }
    exe_time = millis()-pre_time;
    if (exe_time > 0)
    {
      cliPrintf("%d KB/sec\n", 1024 * 1000 / exe_time);
    }
    ret = true;
  }
  if (ret != true)
  {
    cliPrintf("sd info\n");

    if (is_init == true)
    {
      cliPrintf("sd read block_number\n");
      cliPrintf("sd speed-test\n");
    }
  }
}
#endif


#endif
