#ifndef HW_DEF_H
#define HW_DEF_H

#include "def.h"
#include "main.h"


#define _DEF_FIRMWATRE_VERSION "V0"
#define _DEF_BOARD_NAME        "STM32H5_KIT"

#define _USE_HW_GPIO           1
#define HW_GPIO_MAX_CH         GPIO_PIN_MAX

#define _USE_HW_LED
#define HW_LED_MAX_CH 1

typedef enum
{
  LED,
  GPIO_PIN_MAX,
} GpioPinName_t;

void     logPrintf(const char *fmt, ...);
void     delay(uint32_t ms);
uint32_t millis(void);

#endif
