#include "hw.h"
#include "stm32h5xx_hal_eth.h"

#define RESET_DOUBLE_TAP_MAGIC 0xDEADBEEF
#define RESET_DOUBLE_TAP_TIMEOUT 500 // ms

volatile const firm_ver_t firm_ver __attribute__((section(".version"))) =
    {
        .magic_number = VERSION_MAGIC_NUMBER,
        .version_str = _DEF_FIRMWATRE_VERSION,
        .name_str = _DEF_BOARD_NAME,
        .firm_addr = 0x08000000};

static void jumpToSysBoot(void);

bool hwInit(void)
{
  cliInit();
  logInit();
  gpioInit();
  swtimerInit();
  rtcInit();

  // UART 먼저 초기화 (resetInit 내부 cliPrintf를 위해)
  uartInit();
//  for (int i=0; i<UART_MAX_CH; i++)
//  {
//    uartOpen(i, uartGetBaud(i));
//  }
  logOpen(HW_UART_CH_DEBUG, 115200);

  resetInit();

  //-- 더블 리셋 감지 → DFU 진입
  //
  if (resetGetBits() & (1 << RESET_BIT_PIN))
  {
    uint32_t double_tap_flag = 0;
    rtcGetReg(2, &double_tap_flag);

    if (double_tap_flag == RESET_DOUBLE_TAP_MAGIC)
    {
      rtcSetReg(2, 0);
      logPrintf("Double tap detected! Jumping to DFU...\r\n");
      jumpToSysBoot();
    }
    else
    {
      rtcSetReg(2, RESET_DOUBLE_TAP_MAGIC);
      HAL_Delay(RESET_DOUBLE_TAP_TIMEOUT);
      rtcSetReg(2, 0);
    }
  }

  buttonInit();
  ledInit();

  logPrintf("\r\n[ Firmware Begin... ]\r\n");
  logPrintf("Booting..Name \t\t: %s\r\n", _DEF_BOARD_NAME);
  logPrintf("Booting..Ver  \t\t: %s\r\n", _DEF_FIRMWATRE_VERSION);
  logPrintf("Booting..Clock\t\t: %d Mhz\r\n", (int)HAL_RCC_GetSysClockFreq()/1000000);
  logPrintf("\n");

  i2cInit();
  lcdInit();
  lcdSetFps(20);
  pwmInit();
  buzzerInit();
  spiInit();
  spiFlashInit();
  sdInit();
  framInit();
  canInit();

  etnetInit();
  return true;
}


static void jumpToSysBoot(void)
{
  void (*SysMemBootJump)(void);
  volatile uint32_t addr = 0x0BF97000; // STM32H563 시스템 메모리 (AN2606)

  HAL_RCC_DeInit();

  SysTick->CTRL = 0;
  SysTick->LOAD = 0;
  SysTick->VAL = 0;

  __disable_irq();

  for (int i = 0; i < 8; i++)
  {
    NVIC->ICER[i] = 0xFFFFFFFF;
    NVIC->ICPR[i] = 0xFFFFFFFF;
    __DSB();
    __ISB();
  }

  __enable_irq();

  SysMemBootJump = (void (*)(void))(*((uint32_t *)(addr + 4)));
  __set_MSP(*(uint32_t *)addr);
  SysMemBootJump();

  while (1)
    ;
}

void delay(uint32_t ms)
{
  HAL_Delay(ms);
}

uint32_t millis(void)
{
  return HAL_GetTick();
}
