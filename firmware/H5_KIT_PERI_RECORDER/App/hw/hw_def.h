#ifndef HW_DEF_H
#define HW_DEF_H

#include "main.h"
#include "def.h"


#define _DEF_FIRMWATRE_VERSION    "V0"
#define _DEF_BOARD_NAME           "STM32H5_KIT"

#define _USE_HW_ETNET

#define _USE_HW_LED
#define      HW_LED_MAX_CH          1

#define _USE_HW_UART
#define      HW_UART_MAX_CH         9
#define      HW_UART_CH_DEBUG      _DEF_UART1
#define      HW_UART_CH_UART_1     _DEF_UART2
#define      HW_UART_CH_UART_2     _DEF_UART3
#define      HW_UART_CH_RS485_2    _DEF_UART4
#define      HW_UART_CH_RS485_1    _DEF_UART5
#define      HW_UART_CH_LIN_1      _DEF_UART6
#define      HW_UART_CH_LIN_2      _DEF_UART7
#define      HW_UART_CH_LIN_3      _DEF_UART8
#define      HW_UART_CH_RS232      _DEF_UART9


#define _USE_HW_LOG
#define      HW_LOG_CH              HW_UART_CH_DEBUG
#define      HW_LOG_BOOT_BUF_MAX    2048
#define      HW_LOG_LIST_BUF_MAX    4096

#define _USE_HW_CLI
#define      HW_CLI_CMD_LIST_MAX    32
#define      HW_CLI_CMD_NAME_MAX    16
#define      HW_CLI_LINE_HIS_MAX    8
#define      HW_CLI_LINE_BUF_MAX    64

#define _USE_HW_CLI_GUI
#define      HW_CLI_GUI_WIDTH       80
#define      HW_CLI_GUI_HEIGHT      24

#define _USE_HW_BUTTON
#define      HW_BUTTON_MAX_CH       1

#define _USE_HW_SWTIMER
#define      HW_SWTIMER_MAX_CH      8

#define _USE_HW_RTC
#define      HW_RTC_BOOT_MODE       RTC_BKP_DR3
#define      HW_RTC_RESET_BITS      RTC_BKP_DR4

#define _USE_HW_USB
#define _USE_HW_CDC
#define      HW_USE_CDC             1
#define      HW_USE_MSC             0

#define _USE_HW_CAN
#define      HW_CAN_MAX_CH          2
#define      HW_CAN_MSG_RX_BUF_MAX  32

#define _USE_HW_I2C
#define      HW_I2C_MAX_CH          2
#define 	 HW_I2C_OLED			_DEF_I2C1
#define      HW_I2C_CH_FRAM         _DEF_I2C4

#define _USE_HW_FRAM
#define      HW_FRAM_MAX_SIZE       (2*1024*8) // bits

#define _USE_HW_GPIO
#define      HW_GPIO_CH_LIN_EN      0
#define      HW_GPIO_MAX_CH         3

#define _USE_HW_LCD					1
#define _USE_HW_SSD1306				1
#define      HW_LCD_WIDTH           128
#define      HW_LCD_HEIGHT          64

#define _USE_HW_PWM
#define HW_PWM_MAX_CH 1

#define _USE_HW_BUZZER

#define _USE_HW_SPI
#define      HW_SPI_MAX_CH          1

#define _USE_HW_SPI_FLASH
#define      HW_SPI_FLASH_ADDR      0x91000000

#define _USE_CLI_HW_SPI_FLASH       1

#define _USE_HW_SD

typedef enum
{
  LED,
  SPI_FLASH_CS,
  SD_CD,
  GPIO_PIN_MAX,
} GpioPinName_t;

void logPrintf(const char *fmt, ...);
void delay(uint32_t ms);
uint32_t millis(void);

#endif
