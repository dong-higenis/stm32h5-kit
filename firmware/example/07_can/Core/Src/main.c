/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

/**
 * @brief 해당 예제는 CAN 통신을 다뤄볼 예정입니다.
 *        cli에서 can 메시지를 송신하거나 수신해보는 예제코드 입니다.
 *        혹여나 target보드 등이 없어 can 테스트를 해보기 힘드신 분들은
 *        _USE_CAN_LOOPBACK_MODE 를 1로 바꿔주시면 자가 테스트가 가능합니다.
 *
 * @note 해당 예제를 실험하기 위해서 .ioc내 클럭설정에서 CAN이 80MHZ에 맞춰진 환경이어야 합니다.
 *       CANFD를 사용하지만 간단한 실험을 위해 Classic CAN처럼 이용하도록 하겠습니다.
 *       따라서 BRS(Bit Rate Switch(비트 속도 전환))옵션을 끄도록 하겠습니다.
 *
 * @note main파일이 너무 길어지는 관계로 별도의 파일 "custom_can.c" "custom_can.h"를 만들겠습니다.
 */

/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fdcan.h"
#include "gpdma.h"
#include "gpio.h"
#include "i2c.h"
#include "icache.h"
#include "usart.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "cli.h"
#include "custom_can.h" // can include 추가
#include "ssd1306.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define _USE_CAN_LOOPBACK_MODE 0 // CAN loopback 사용 여부

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

static uint8_t debug_uart_rx_char;
static bool    cli_led_toggle_req;
static bool    oled_block_mode_req = false;
static bool    can_block_mode_req  = false; // CAN 블로킹 플래그 추가

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart5;
extern UART_HandleTypeDef huart6;
extern UART_HandleTypeDef huart7;
extern UART_HandleTypeDef huart10;

extern FDCAN_HandleTypeDef hfdcan1; // CAN 핸들러 추가
extern FDCAN_HandleTypeDef hfdcan2; // CAN 핸들러 추가

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
/* USER CODE BEGIN PFP */

/**
 * @brief 각각의 기능에 대한 함수를 만들어 줍니다.
 */
void cliLed(cli_args_t *args);
void cliUart6(cli_args_t *args);
void cliUart10(cli_args_t *args);
void cliRs232(cli_args_t *args);
void cliRs485(cli_args_t *args);
void cliOled(cli_args_t *args);
void cliCan(cli_args_t *args); // CAN 명령어 추가

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
 * @brief uart 수신 콜백함수에서 cliProcess에 바로 문자를 하나씩 넘겨줍니다.
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart == &huart1)
  {
    // OLED나 CAN 블로킹 모드가 아닐 때만 CLI 처리
    if (!oled_block_mode_req && !can_block_mode_req)
    {
      cliProcess(debug_uart_rx_char);
      HAL_UART_Receive_IT(&huart1, &debug_uart_rx_char, 1);
    }
  }
}

/**
 * @brief arduino와 같은 함수명 (선택 사항)
 */
uint32_t millis(void)
{
  return HAL_GetTick();
}

/**
 * @brief led처리 함수 : 사용법 (ex : "led toggle")
 */
void cliLed(cli_args_t *args)
{
  if (args->argc < 1)
  {
    cliPrintf("Usage: led on\r\n");
    cliPrintf("Usage: led off\r\n");
    cliPrintf("Usage: led toggle\r\n");
    return;
  }

  if (strcmp(args->argv[0], "on") == 0)
  {
    if (cli_led_toggle_req)
    {
      cli_led_toggle_req = false;
    }
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
    cliPrintf("LED ON\r\n");
  }
  else if (strcmp(args->argv[0], "off") == 0)
  {
    if (cli_led_toggle_req)
    {
      cli_led_toggle_req = false;
    }
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
    cliPrintf("LED OFF\r\n");
  }
  else if (strcmp(args->argv[0], "toggle") == 0)
  {
    cli_led_toggle_req = true;
  }
  else
  {
    cliPrintf("Error: use 'on' or 'off'\r\n");
  }
}

/**
 * @brief uart send 함수 : 사용법 (ex : "uart6")
 */
void cliUart6(cli_args_t *args)
{
  char msg[] = "[SENT]Hello Uart!\r\n";
  HAL_UART_Transmit(&huart6, (uint8_t *)msg, strlen(msg), 100);
  cliPrintf("UART6 transmitted\r\n");
}

/**
 * @brief uart send 함수 : 사용법 (ex : "uart10")
 */
void cliUart10(cli_args_t *args)
{
  char msg[] = "[SENT]Hello Uart!\r\n";
  HAL_UART_Transmit(&huart10, (uint8_t *)msg, strlen(msg), 100);
  cliPrintf("UART10 transmitted\r\n");
}

/**
 * @brief rs232 send 함수 : 사용법 (ex : "rs232")
 */
void cliRs232(cli_args_t *args)
{
  char msg[] = "[RS232]Hello Uart!\r\n";
  HAL_UART_Transmit(&huart7, (uint8_t *)msg, strlen(msg), 100);
  cliPrintf("RS232 transmitted\r\n");
}

/**
 * @brief rs485 send 함수 : 사용법 (ex : "rs485 1")
 */
void cliRs485(cli_args_t *args)
{
  if (args->argc < 1)
  {
    cliPrintf("Usage: rs485 1 or 2\r\n");
    return;
  }

  if (strcmp(args->argv[0], "1") == 0)
  {
    HAL_GPIO_WritePin(UART5_DE_GPIO_Port, UART5_DE_Pin, GPIO_PIN_SET);

    char msg[] = "[RS485-1]Hello Uart!\r\n";
    HAL_UART_Transmit(&huart5, (uint8_t *)msg, strlen(msg), 100);

    while (__HAL_UART_GET_FLAG(&huart5, UART_FLAG_TC) == RESET)
      ;

    HAL_GPIO_WritePin(UART5_DE_GPIO_Port, UART5_DE_Pin, GPIO_PIN_RESET);
    cliPrintf("RS485-1 transmitted\r\n");
  }
  else if (strcmp(args->argv[0], "2") == 0)
  {
    HAL_GPIO_WritePin(UART4_DE_GPIO_Port, UART4_DE_Pin, GPIO_PIN_SET);

    char msg[] = "[RS485-2]Hello Uart!\r\n";
    HAL_UART_Transmit(&huart4, (uint8_t *)msg, strlen(msg), 100);

    while (__HAL_UART_GET_FLAG(&huart4, UART_FLAG_TC) == RESET)
      ;

    HAL_GPIO_WritePin(UART4_DE_GPIO_Port, UART4_DE_Pin, GPIO_PIN_RESET);
    cliPrintf("RS485-2 transmitted\r\n");
  }
  else
  {
    cliPrintf("Error: use '1' or '2'\r\n");
  }
}

/**
 * @brief oled에 문자열을 출력하는 함수
 *        (사용법 : oled test 입력 후 실시간으로 타이핑 시 oled에 입력)
 *        (나갈때는 Enter)
 */
void cliOled(cli_args_t *args)
{
  if (args->argc < 1 || strcmp(args->argv[0], "test") != 0)
  {
    cliPrintf("Usage: oled test\r\n");
    return;
  }

  oled_block_mode_req = true;

  HAL_UART_AbortReceive_IT(&huart1);

  ssd1306Clear();
  ssd1306SetCursor(0, 0);
  ssd1306UpdateScreen();

  uint8_t ch;

  while (1)
  {
    HAL_StatusTypeDef ret = HAL_UART_Receive(&huart1, &ch, 1, 10);

    if (ret == HAL_OK)
    {
      if (ch == '\r' || ch == '\n')
      {
        ssd1306Clear();
        ssd1306UpdateScreen();
        break;
      }
      else
      {
        ssd1306WriteChar((char)ch);
        ssd1306UpdateScreen();
      }
    }
  }

  oled_block_mode_req = false;
  HAL_UART_Receive_IT(&huart1, &debug_uart_rx_char, 1);
}

/**
 * @brief CAN 테스트 명령어
 *        사용법: can test 1 (CAN1 테스트)
 *              can test 2 (CAN2 테스트)
 */
void cliCan(cli_args_t *args)
{
  // 사용법 출력
  if (args->argc < 2)
  {
    cliPrintf("Usage: can test ch\r\n");
    cliPrintf("ch: [1 ~ 2]\r\n");
    return;
  }

  // 명령어 확인
  if (strcmp(args->argv[0], "test") != 0)
  {
    cliPrintf("Unknown command. Use 'can test ch'\r\n");
    return;
  }

  // 채널 파싱
  uint8_t ch = 0;
  if (args->argc >= 2 && strlen(args->argv[1]) == 1)
  {
    if (args->argv[1][0] == '1')
    {
      ch = 1;
    }
    else if (args->argv[1][0] == '2')
    {
      ch = 2;
    }
  }
  else
  {
    cliPrintf("Error: Channel must be 1 or 2\r\n");
    return;
  }

  if (ch == 0)
  {
    cliPrintf("Error: Invalid channel format\r\n");
    return;
  }

  // CAN 핸들러 선택
  FDCAN_HandleTypeDef *p_can = (ch == 1) ? &hfdcan1 : &hfdcan2;

#if _USE_CAN_LOOPBACK_MODE
  if (!customCanInit(p_can, true))           // 루프백 모드
  {
    cliPrintf("Error: CAN init failed\r\n"); // can 초기화 실패
    return;
  }
  cliPrintf("CAN%d started in LOOPBACK mode\r\n", ch);
#else
  if (!customCanInit(p_can, false)) // 정상 모드
  {
    cliPrintf("Error: CAN init failed\r\n");
    return;
  }
  cliPrintf("CAN%d started in NORMAL mode\r\n", ch);
#endif

  // 블로킹 모드 진입
  can_block_mode_req = true;
  HAL_UART_AbortReceive_IT(&huart1); // 인터럽트 중지

  uint8_t uart_rx_data;

  while (1)
  {
    // 폴링 방식으로 키보드 입력 대기
    HAL_StatusTypeDef ret = HAL_UART_Receive(&huart1, &uart_rx_data, 1, 10);

    if (ret == HAL_OK)
    {
      // Enter 입력 시 종료
      if (uart_rx_data == '\r' || uart_rx_data == '\n')
      {
        break;
      }

      // CAN 메시지 전송
      FDCAN_TxHeaderTypeDef tx_header;
      uint8_t               tx_data[8] = {0};

      // 첫 바이트에 사용자 입력 문자의 16진수 값 저장
      tx_data[0] = uart_rx_data;

      // TX 헤더 설정
      tx_header.Identifier          = 0x123;             // CAN ID (예시)
      tx_header.IdType              = FDCAN_STANDARD_ID; // 표준 ID (11비트)
      tx_header.TxFrameType         = FDCAN_DATA_FRAME;  // 데이터 프레임
      tx_header.DataLength          = FDCAN_DLC_BYTES_8; // 8바이트
      tx_header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
      tx_header.BitRateSwitch       = FDCAN_BRS_OFF;     // BRS 끄기
      tx_header.FDFormat            = FDCAN_CLASSIC_CAN; // Classic CAN
      tx_header.TxEventFifoControl  = FDCAN_NO_TX_EVENTS;
      tx_header.MessageMarker       = 0;

      // 메시지 전송
      if (HAL_FDCAN_AddMessageToTxFifoQ(p_can, &tx_header, tx_data) != HAL_OK)
      {
        cliPrintf("[TX] Error: Failed to send\r\n");
      }
      else
      {
        cliPrintf("[TX] Char:'%c' (0x%02X) sent\r\n",
                  (uart_rx_data >= 32 && uart_rx_data <= 126) ? uart_rx_data : '.',
                  uart_rx_data);
      }

      uint32_t rx_fifo = HAL_FDCAN_GetRxFifoFillLevel(p_can, FDCAN_RX_FIFO0);


      // RX FIFO 비우기
      while (rx_fifo > 0)
      {
        FDCAN_RxHeaderTypeDef rx_header;
        uint8_t               rx_data[8];

        if (HAL_FDCAN_GetRxMessage(p_can, FDCAN_RX_FIFO0, &rx_header, rx_data) == HAL_OK)
        {
          cliPrintf(" [RX] ID:0x%03lX 0x%02X '%c'\r\n",
                    rx_header.Identifier, rx_data[0],
                    (rx_data[0] >= 32 && rx_data[0] <= 126) ? rx_data[0] : '.');
        }
        rx_fifo--;
      }
    }
  }

  // CAN 종료
  customCanClose(p_can);

  // 블로킹 모드 해제
  can_block_mode_req = false;
  HAL_UART_Receive_IT(&huart1, &debug_uart_rx_char, 1); // uart 인터럽트 재활성화
}

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_GPDMA1_Init();
  MX_USART1_UART_Init();
  MX_USART6_UART_Init();
  MX_USART10_UART_Init();
  MX_ICACHE_Init();
  MX_UART7_Init();
  MX_UART4_Init();
  MX_UART5_Init();
  MX_I2C1_Init();
  MX_FDCAN1_Init();
  MX_FDCAN2_Init();
  /* USER CODE BEGIN 2 */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);

  HAL_UART_Receive_IT(&huart1, &debug_uart_rx_char, 1);

  HAL_GPIO_WritePin(UART4_DE_GPIO_Port, UART4_DE_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(UART5_DE_GPIO_Port, UART5_DE_Pin, GPIO_PIN_RESET);

  // cli 초기화 및 명령을 등록 해줍시다.
  cliInit();
  cliAdd("led", cliLed);
  cliAdd("uart6", cliUart6);
  cliAdd("uart10", cliUart10);
  cliAdd("rs232", cliRs232);
  cliAdd("rs485", cliRs485);
  cliAdd("oled", cliOled);
  cliAdd("can", cliCan); // can 추가

  ssd1306Init();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {
    if (cli_led_toggle_req)
    {
      static uint32_t pre_time;
      static bool     current_led_state = true; // off

      if (millis() - pre_time > 500)
      {
        pre_time          = millis();
        current_led_state = !current_led_state;
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, current_led_state);
      }
    }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }

  /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
   */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY))
  {
  }

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
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

  /** Initializes the CPU, AHB and APB buses clocks
   */
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

  /** Configure the programming delay
   */
  __HAL_FLASH_SET_PROGRAM_DELAY(FLASH_PROGRAMMING_DELAY_2);
}

/**
 * @brief Peripherals Common Clock Configuration
 * @retval None
 */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
   */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;
  PeriphClkInitStruct.PLL2.PLL2Source      = RCC_PLL2_SOURCE_HSE;
  PeriphClkInitStruct.PLL2.PLL2M           = 5;
  PeriphClkInitStruct.PLL2.PLL2N           = 32;
  PeriphClkInitStruct.PLL2.PLL2P           = 2;
  PeriphClkInitStruct.PLL2.PLL2Q           = 2;
  PeriphClkInitStruct.PLL2.PLL2R           = 2;
  PeriphClkInitStruct.PLL2.PLL2RGE         = RCC_PLL2_VCIRANGE_2;
  PeriphClkInitStruct.PLL2.PLL2VCOSEL      = RCC_PLL2_VCORANGE_WIDE;
  PeriphClkInitStruct.PLL2.PLL2FRACN       = 0;
  PeriphClkInitStruct.PLL2.PLL2ClockOut    = RCC_PLL2_DIVQ;
  PeriphClkInitStruct.FdcanClockSelection  = RCC_FDCANCLKSOURCE_PLL2Q;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */


/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
