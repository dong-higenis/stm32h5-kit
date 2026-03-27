#ifndef HW_H_
#define HW_H_


#include "hw_def.h"

#include "led.h"
#include "uart.h"
#include "common/hw/include/gpio.h"
#include "swtimer.h"
#include "cli.h"
#include "rtc.h"
#include "common/hw/include/i2c.h"
#include "common/hw/include/spi.h"
#include "common/hw/include/rtc.h"
#include "reset.h"
#include "spi_flash.h"
#include "lcd.h"
//#include "flash.h"
#include "button.h"
#include "log.h"
#include "sd.h"
//#include "ws2812.h"
#include "can.h"
#include "pwm.h"
#include "buzzer.h"
#include "fram.h"
#include "etnet.h"


bool hwInit(void);

#endif
