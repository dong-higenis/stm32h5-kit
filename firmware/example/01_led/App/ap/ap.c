#include "ap.h"
#include "stm32h5xx_hal.h"

/**
 * @brief 향후 app개발에 필요한 모듈들 초기화
 */
void apInit(void)
{

}

/**
 * @brief 500ms마다 led blink
 */
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
  }
}
