#include "hw.h"
#include "stm32h5xx_hal_eth.h"



volatile const firm_ver_t firm_ver __attribute__((section(".version"))) =
{
  .magic_number = VERSION_MAGIC_NUMBER,
  .version_str  = _DEF_FIRMWATRE_VERSION,
  .name_str     = _DEF_BOARD_NAME,
  .firm_addr    = 0x08000000
};

bool hwInit(void)
{
  cliInit();
  logInit();
  gpioInit();
  swtimerInit();
  rtcInit();
  buttonInit();
  ledInit();
  uartInit();
  for (int i=0; i<UART_MAX_CH; i++)
  {
    uartOpen(i, uartGetBaud(i));
  }

  logOpen(HW_UART_CH_DEBUG, 115200);
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

//  etnetInit();
  return true;
}

void delay(uint32_t ms)
{
  HAL_Delay(ms);
}
uint32_t millis(void)
{
  return HAL_GetTick();
}
