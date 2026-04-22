#ifndef HW_DEF_H_
#define HW_DEF_H_



#include "bsp.h"
#include "assert_def.h"


#define _DEF_FIRMWATRE_VERSION    "V250617R1"
#define _DEF_BOARD_NAME           "STM32H5-FW"



#define _USE_HW_ASSERT
#define _USE_HW_FAULT
#define _USE_HW_BUZZER
#define _USE_HW_MICROS
#define _USE_HW_NVS
#define _USE_HW_RTOS


#define _USE_HW_LED
#define      HW_LED_MAX_CH          1

#define _USE_HW_UART
#define      HW_UART_MAX_CH         9
#define      HW_UART_CH_DEBUG      _DEF_UART1
#define      HW_UART_CH_UART_1     _DEF_UART2
#define      HW_UART_CH_UART_2     _DEF_UART3 
#define      HW_UART_CH_RS232      _DEF_UART4 
#define      HW_UART_CH_RS485_1    _DEF_UART5
#define      HW_UART_CH_RS485_2    _DEF_UART6
#define      HW_UART_CH_LIN_1      _DEF_UART7
#define      HW_UART_CH_LIN_2      _DEF_UART8
#define      HW_UART_CH_LIN_3      _DEF_UART9


#define _USE_HW_CLI
#define      HW_CLI_CMD_LIST_MAX    32
#define      HW_CLI_CMD_NAME_MAX    16
#define      HW_CLI_LINE_HIS_MAX    8
#define      HW_CLI_LINE_BUF_MAX    64

#define _USE_HW_CLI_GUI
#define      HW_CLI_GUI_WIDTH       80
#define      HW_CLI_GUI_HEIGHT      24

#define _USE_HW_LOG
#define      HW_LOG_CH              HW_UART_CH_DEBUG
#define      HW_LOG_BOOT_BUF_MAX    2048
#define      HW_LOG_LIST_BUF_MAX    4096

#define _USE_HW_SWTIMER
#define      HW_SWTIMER_MAX_CH      16

#define _USE_HW_BUTTON
#define      HW_BUTTON_MAX_CH       BUTTON_PIN_MAX

#define _USE_HW_RESET
#define      HW_RESET_BOOT          1

#define _USE_HW_RTC
#define      HW_RTC_BOOT_MODE       RTC_BKP_DR3
#define      HW_RTC_RESET_BITS      RTC_BKP_DR4

#define _USE_HW_I2C
#define      HW_I2C_MAX_CH          1

#define _USE_HW_LCD
#define      HW_LCD_LVGL            0
#define _USE_HW_SSD1306
#define      HW_SSD1306_MODEL       1
#define      HW_LCD_WIDTH           128
#define      HW_LCD_HEIGHT          64

#define _USE_HW_GPIO
#define      HW_GPIO_MAX_CH         GPIO_PIN_MAX

#define _USE_HW_SPI
#define      HW_SPI_MAX_CH          1

#define _USE_HW_SPI_FLASH
#define      HW_SPI_FLASH_ADDR      0x91000000

#define _USE_HW_PWM
#define      HW_PWM_MAX_CH          PWM_PIN_MAX


#define _USE_HW_FS
#define      HW_FS_FLASH_OFFSET     0   
#define      HW_FS_MAX_SIZE         (8*1024*1024)

#define _USE_HW_CAN
#define      HW_CAN_MAX_CH          2
#define      HW_CAN_MSG_RX_BUF_MAX  32

#define _USE_HW_SD
#define _USE_HW_FATFS
#define _USE_HW_ETNET
#define _USE_HW_ETNET_UDP    


//-- CLI
//
#define _USE_CLI_HW_LOG             1
#define _USE_CLI_HW_ASSERT          1
#define _USE_CLI_HW_UART            1
#define _USE_CLI_HW_ESP             1


//-- RTOS
//
#define _USE_HW_THREAD
#define      HW_THREAD_MAX_CNT                  7

#define _HW_DEF_THREAD_MAIN_PRI                 osPriorityNormal
#define _HW_DEF_THREAD_MAIN_STACK               (8*1024)

#define _HW_DEF_THREAD_CLI_PRI                  osPriorityNormal
#define _HW_DEF_THREAD_CLI_STACK                (32*1024)

#define _HW_DEF_THREAD_PERI_PRI                 osPriorityNormal
#define _HW_DEF_THREAD_PERI_STACK               (32*1024)

#define _HW_DEF_THREAD_WRITER_PRI               osPriorityNormal
#define _HW_DEF_THREAD_WRITER_STACK             (32*1024)

#define _HW_DEF_THREAD_EVENT_PRI                osPriorityNormal
#define _HW_DEF_THREAD_EVENT_STACK              (8*1024)

#define _HW_DEF_THREAD_MQTT_PRI                 osPriorityNormal
#define _HW_DEF_THREAD_MQTT_STACK               (8*1024)

#define _HW_DEF_THREAD_WIFI_PRI                 osPriorityNormal
#define _HW_DEF_THREAD_WIFI_STACK               (8*1024)

#define _HW_DEF_THREAD_RELAY_PRI                osPriorityNormal
#define _HW_DEF_THREAD_RELAY_STACK              (8*1024)

#define _HW_DEF_THREAD_MOTOR_PRI                osPriorityNormal
#define _HW_DEF_THREAD_MOTOR_STACK              (8*1024)


typedef enum
{
  NORMAL_BTN,

  BUTTON_PIN_MAX
} ButtonPinName_t;

typedef enum
{
  SPI_FLASH_CS,
  SD_DETECT,

  GPIO_PIN_MAX
} GpioPinName_t;

typedef enum
{
  DC_EN,
  PWM_PIN_MAX,
} PwmPinName_t;

#endif
