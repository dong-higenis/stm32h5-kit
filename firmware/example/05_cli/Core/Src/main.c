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
 * @brief 해당 예제는 이전 예제를 종합하여,
 *        개발에 편한 테스트를 위해 CLI를 구현합니다.
 *        Src폴더 아래에 cli.c 그리고 Inc폴더 아래에 cli.h를 만들어주고 구현을 시작합니다.
 *
 * @details cli 구현이 끝나면 해당 main파일에서, 각각의 기능에 대한 함수를 추가하고,
 *          cli의 명령리스트에 추가해 줍시다.
 *          작업이 끝나면 실제로 빌드 해보고, 업로드 하여
 *          tere term에서 cli가 잘 동작하는지 확인 해줍시다.
 *
 *          help를 입력하여 새로 정의한 함수가 목록에 잘뜨는지 확인 해줍시다.
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "gpdma.h"
#include "gpio.h"
#include "icache.h"
#include "usart.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include <string.h>

#include "cli.h" // cli 추가
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

//#define RX_BUF_SIZE 64

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

static uint8_t          debug_uart_rx_char;
static bool             cli_led_toggle_req; // led 토글 플래그
// 이제 아래 변수는 쓰지 않아도 됩니다.
// static char             debug_uart_rx_line[RX_BUF_SIZE];
// static volatile uint8_t debug_uart_rx_idx   = 0;
// static volatile bool    debug_uart_rx_ready = false;

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart5;
extern UART_HandleTypeDef huart6;
extern UART_HandleTypeDef huart7;
extern UART_HandleTypeDef huart10;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/**
 * @brief 각각의 기능에 대한 함수를 만들어 줍니다.
 */
void cliLed(cli_args_t *args);
void cliUart6(cli_args_t *args);
void cliUart10(cli_args_t *args);
void cliRs232(cli_args_t *args);
void cliRs485(cli_args_t *args);

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
    cliProcess(debug_uart_rx_char);
    HAL_UART_Receive_IT(&huart1, &debug_uart_rx_char, 1);
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

  if (strcmp(args->argv[0], "on") == 0) // 첫번째 인자
  {
    if (cli_led_toggle_req) // 무한 토글 방지
    {
      cli_led_toggle_req = false;
    }
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
    cliPrintf("LED ON\r\n");
  }
  else if (strcmp(args->argv[0], "off") == 0)
  {
	if (cli_led_toggle_req) // 무한 토글 방지
	{
	  cli_led_toggle_req = false;
	}
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
    cliPrintf("LED OFF\r\n");
  }
  else if (strcmp(args->argv[0], "toggle") == 0) // 토글 기능 추가
  {
	  cli_led_toggle_req = true; // while문에 전달
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

    while (__HAL_UART_GET_FLAG(&huart5, UART_FLAG_TC) == RESET);

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

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {
	if (cli_led_toggle_req)
	{
	  static uint32_t pre_time;
	  static bool     current_led_state = true;    // off

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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY))
  {
  }

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv              = RCC_HSI_DIV2;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_PCLK3;
  RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the programming delay
   */
  __HAL_FLASH_SET_PROGRAM_DELAY(FLASH_PROGRAMMING_DELAY_0);
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
