#include "ap.h"
#include "stm32h5xx_hal.h"

void apInit(void)
{
  cliOpen(HW_UART_CH_DEBUG, 115200);
  logBoot(true);
}

void apMain(void)
{
  static uint32_t pre_time = 0;

  while (1)
  {
    if (millis() - pre_time > 500)
    {
        pre_time = millis();
        ledToggle(_DEF_LED1);
    }
    cliMain();

    sdUpdate();

    etnetUpdate();
    tx_thread_sleep(1);
  }
}
