#include "hw.h"

/**
 * @brief 하드웨어 초기화
 */
bool hwInit(void)
{
  ledInit();  // led 초기화
  uartInit(); // uart 초기화
  return true;
}

/**
 * @brief 시간 관련 wrapping 함수들
 */
void delay(uint32_t ms)
{
  HAL_Delay(ms);
}

uint32_t millis(void)
{
  return HAL_GetTick();
}
