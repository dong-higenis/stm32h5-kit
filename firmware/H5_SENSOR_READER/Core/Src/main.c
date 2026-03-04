/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 */
/**
 * @brief 해당 예제는, rs485 한 채널을 사용하여,
 *        두 센서와 통신하는 간단한 예제입니다.
 *        .ioc에서 설정할 값은 UART1(디버그용), UART4, UART5 등이 있으며,
 *        자세한 설정은 해당 예제 파일의 .ioc파일을 열어 확인 해주십시오.
 * 
 * @note  이해를 돕기 위해,
 *        oled의 "ssd1306" 드라이버 파일을 제외하면,
 *        main.c파일에서만 작성이 되어 있습니다.
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "gpio.h"
#include "i2c.h"
#include "icache.h"
#include "usart.h"

/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "rain_sensor.h"
#include "ssd1306.h"
#include "wind_sensor.h"
/* USER CODE END Includes */

/* USER CODE BEGIN PD */

/**
 * @brief 디버깅용 printf 사용 여부
 */
#define USE_DEBUG_PRINTF    1

/**
 * @brief 해당 예제 사용 rs485 uart 채널
 */
#define USE_RS485_UART_CH_4 0
#define USE_RS485_UART_CH_5 1

/**
 * @brief rs485 송신 주기 (ms)
 */
#define RS485_TX_PERIOD     300

/**
 * @brief rs485 수신 타임아웃 (ms)
 */
#define RS485_RX_TIMEOUT    1000

/**
 * @brief oled 갱신 주기 (ms)
 */
#define OLED_UPDATE_PERIOD  100

/**
 * @brief 디버그 출력 주기 (ms)
 */
#define DEBUG_PRINT_PERIOD  500
/* USER CODE END PD */

/* USER CODE BEGIN PV */

/**
 * @brief 사용 uart ch에 따른 핀 정의
 */
#if USE_RS485_UART_CH_4
extern UART_HandleTypeDef huart4;
#define RS485_UART    huart4
#define RS485_DE_PORT U4_DE_GPIO_Port
#define RS485_DE_PIN  U4_DE_Pin
#endif

#if USE_RS485_UART_CH_5
extern UART_HandleTypeDef huart5;
#define RS485_UART    huart5
#define RS485_DE_PORT U5_DE_GPIO_Port
#define RS485_DE_PIN  U5_DE_Pin
#endif

#if USE_DEBUG_PRINTF
extern UART_HandleTypeDef huart1;
#endif

/**
 * @brief 센서 요청 / 응답 구조체
 */
wind_sensor_req_t  wind_sensor_req;
wind_sensor_resp_t wind_sensor_resp;

rain_sensor_req_t  rain_sensor_req;
rain_sensor_resp_t rain_sensor_resp;

/**
 * @brief 센서 측정 값
 */
float wind_speed = 0.0f;
float rain_speed = 0.0f;

/**
 * @brief 수신 버퍼
 */
uint8_t rx_buf[32];

/**
 * @brief req대상 센서 토글 플래그
 */
static bool is_rain_turn = false;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static uint16_t modbus_crc16(uint8_t *buf, int len);
static bool     sensorRequest(bool is_rain);
static void     sensorParse(bool is_rain);
static void     oledUpdate(void);

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/**
 * @brief 센서 요청
 * @return true: 수신 성공, false: 타임아웃
 */
static bool sensorRequest(bool is_rain)
{
  uint8_t *p_req;
  uint16_t tx_size;
  uint16_t rx_size;

  // 현재 루프의 타겟 센서 확인 -> 대상 센서 포인터와 사이즈를 설정
  if (is_rain)
  {
    p_req  = (uint8_t *)&rain_sensor_req;
    tx_size = sizeof(rain_sensor_req_t);
    rx_size = sizeof(rain_sensor_resp_t);
  }
  else
  {
    p_req  = (uint8_t *)&wind_sensor_req;
    tx_size = sizeof(wind_sensor_req_t);
    rx_size = sizeof(wind_sensor_resp_t);
  }

  // DE 핀 HIGH -> 송신 모드로 전환
  HAL_GPIO_WritePin(RS485_DE_PORT, RS485_DE_PIN, GPIO_PIN_SET);
  HAL_UART_Transmit(&RS485_UART, p_req, tx_size, 100);
  
  // 마지막 바이트까지 물리적으로 전송 완료될 때까지 대기
  while (__HAL_UART_GET_FLAG(&RS485_UART, UART_FLAG_TC) == RESET);

  // DE 핀 LOW -> 수신 모드로 전환
  HAL_GPIO_WritePin(RS485_DE_PORT, RS485_DE_PIN, GPIO_PIN_RESET);

  // 직접 수신한다.
  uint16_t rx_index = 0;
  uint32_t pre_time  = HAL_GetTick();
  
  // 타임 아웃 경과시간 까지 while문 내에서 동작
  while (HAL_GetTick() - pre_time < RS485_RX_TIMEOUT)
  {
    // 수신 데이터 레지스터 내 플래그값이 "데이터 있음(USART_ISR_RXNE_RXFNE)" 이면 true
    if (RS485_UART.Instance->ISR & USART_ISR_RXNE_RXFNE)
    {
      // 한 바이트씩 read한다.
      rx_buf[rx_index++] = (uint8_t)RS485_UART.Instance->RDR;
      // 예상 응답 크기만큼 수신 완료 시 성공 리턴
      if (rx_index >= rx_size)
      {
        return true;
      }
    }
  }

  return false;
}

/**
 * @brief 수신 패킷 파싱
 */
static void sensorParse(bool is_rain)
{
  if (is_rain)
  {
    // rx_buf에 들어있는 수신 패킷을 resp 구조체에 담는다.
    memcpy(&rain_sensor_resp, rx_buf, sizeof(rain_sensor_resp));
    // crc를 계산한다.
    if (modbus_crc16((uint8_t *)&rain_sensor_resp, 5) == rain_sensor_resp.crc)
    {
      // big endian으로 변환한다.
      uint16_t raw = __builtin_bswap16(rain_sensor_resp.rain_val);
      // raw값으로 부터 실제 값을 추출한다.
      rain_speed   = raw * 0.1f;
    }
  }
  else
  {
    memcpy(&wind_sensor_resp, rx_buf, sizeof(wind_sensor_resp));

    if (modbus_crc16((uint8_t *)&wind_sensor_resp, 5) == wind_sensor_resp.crc)
    {
      uint16_t raw = __builtin_bswap16(wind_sensor_resp.wind_val);
      wind_speed   = raw * 0.1f;
    }
  }
}

/**
 * @brief OLED 갱신
 */
static void oledUpdate(void)
{
  char buf[32];
  
  // 전체 화면을 검정색으로 칠한다.
  ssd1306Fill(0x00);

  // 고정 문자열을 출력한다.
  ssd1306SetCursor(0, 0);
  sprintf(buf, "Rain:%5.1f mm/h", rain_speed);
  ssd1306WriteString(buf);

  ssd1306SetCursor(0, 16);
  sprintf(buf, "Wind:%5.1f m/s", wind_speed);
  ssd1306WriteString(buf);
  
  // i2c 전송
  ssd1306UpdateScreen();
}

/**
 * @brief Modbus CRC16
 */
static uint16_t modbus_crc16(uint8_t *buf, int len)
{
  uint16_t crc = 0xFFFF;

  for (int pos = 0; pos < len; pos++)
  {
    crc ^= (uint16_t)buf[pos];

    for (int i = 0; i < 8; i++)
    {
      if (crc & 0x0001)
      {
        crc >>= 1;
        crc  ^= 0xA001;
      }
      else
      {
        crc >>= 1;
      }
    }
  }

  return crc;
}

#if USE_DEBUG_PRINTF // printf를 사용하기 위한 uart1 래핑 함수
int _write(int file, char *ptr, int len)
{
  (void)file;
  HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, HAL_MAX_DELAY);
  return len;
}
#endif

/* USER CODE END 0 */

int main(void)
{
  HAL_Init();
  SystemClock_Config();

  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_USART6_UART_Init();
  MX_USART10_UART_Init();
  MX_ICACHE_Init();
  MX_UART7_Init();
  MX_UART4_Init();
  MX_UART5_Init();
  MX_I2C1_Init();

  /* USER CODE BEGIN 2 */
  
  // 1). DE -> LOW (수신 모드)
  HAL_GPIO_WritePin(RS485_DE_PORT, RS485_DE_PIN, GPIO_PIN_RESET);

  // 2). 풍량 센서 구조체 초기화 및 crc 계산
  windSensorInit(&wind_sensor_req);
  wind_sensor_req.crc = modbus_crc16((uint8_t *)&wind_sensor_req, 6);

  // 3). 강우 센서 구조체 초기화 및 crc 계산
  rainSensorInit(&rain_sensor_req);
  rain_sensor_req.crc = modbus_crc16((uint8_t *)&rain_sensor_req, 6);

  // 4). oled 초기화
  ssd1306Init();

  /* USER CODE END 2 */

  while (1)
  {
    // rs485 
    static uint32_t pre_time = 0;
    
    if (HAL_GetTick() - pre_time > RS485_TX_PERIOD)
    {
      pre_time = HAL_GetTick();
      
      // 센서에게 데이터 요청 (두 센서를 번갈아가며 요청하기 위해, 현재 차례에 대한 정보를 보낸다.)
      if (sensorRequest(is_rain_turn))
      {
        // 데이터 추출 후 전역 변수에 저장
        sensorParse(is_rain_turn);

#if USE_DEBUG_PRINTF // success 디버그 출력
        printf("ok [%s]\r\n", is_rain_turn ? "rain" : "wind");
#endif

      }
      else
      {

#if USE_DEBUG_PRINTF // timeout 디버그 출력
        printf("timeout [%s]\r\n", is_rain_turn ? "rain" : "wind"); 
#endif

      }
      
      // 토글 해준다.
      is_rain_turn = !is_rain_turn;
    }

    // oled
    static uint32_t oled_pre_time;
    if (HAL_GetTick() - oled_pre_time > OLED_UPDATE_PERIOD)
    {
      oled_pre_time = HAL_GetTick();
      oledUpdate();
    }
    
    // debug printf
#if USE_DEBUG_PRINTF
    static uint32_t debug_pre_time;
    if (HAL_GetTick() - debug_pre_time > DEBUG_PRINT_PERIOD)
    {
      debug_pre_time = HAL_GetTick();
      printf("wind: %.1f m/s | rain: %.1f mm/h\r\n", wind_speed, rain_speed);
    }
#endif
  }
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY))
  {
  }

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState       = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource  = RCC_PLL1_SOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM       = 2;
  RCC_OscInitStruct.PLL.PLLN       = 40;
  RCC_OscInitStruct.PLL.PLLP       = 2;
  RCC_OscInitStruct.PLL.PLLQ       = 2;
  RCC_OscInitStruct.PLL.PLLR       = 2;
  RCC_OscInitStruct.PLL.PLLRGE     = RCC_PLL1_VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL  = RCC_PLL1_VCORANGE_WIDE;
  RCC_OscInitStruct.PLL.PLLFRACN   = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_PCLK3;
  RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }

  __HAL_FLASH_SET_PROGRAM_DELAY(FLASH_PROGRAMMING_DELAY_2);
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif
