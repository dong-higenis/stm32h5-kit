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
 * @brief 해당 예제는 stm32h5-kit의 3개 채널 uart를 동시에 다루는 예제입니다.
 *        디버그쪽 Uart(STLink) 포트를 이용하여, 간단한 인터페이스를 구현하며,
 *        Uart를 쉽게 이해 할 수 있게 구현되어 있습니다.
 *        보드의 다른 UART 2개 채널에서도 메시지를 실시간으로 받으며,
 *        500ms마다 "Hello World\n"를 전송합니다.
 *
 * @details 시나리오는 아래와 같습니다.
 *    1. uart 메시지는 Interrupt로 받습니다.
 *    2. Interrupt가 발생할때 마다 (실제 물리적인 uart 메시지가 올때마다), 불리게 되는 Callback 함수 내에서
 *       배열의 초기화, 및 다음 메시지 수신 준비 등 기본적인 과정을 거친 후,
 *       main loop에서 메시지를 파싱하여, 명령을 수행합니다.
 *    3. 명령의 기능적 종류는 세가지이며, LED 토글과, 다른 UART채널로 고정적인 메시지를 보내는것입니다.
 *
 * @note 인터럽트와 메인루프가 공유하는 변수는 volitile을 사용해야 합니다.
 *       인터럽트로 인한 변수값 변화는 컴파일러가 인지하지 못하기 때문에,
 *       뜻하지 않는 캐싱으로 작용 할 수 있음.
 *
 *       ioc에서..
 *       USART1번, 6번, 10번을 활성화 하고 아래 핀번호를 확인합니다.
 *       LED  :  PC13
 *
 *       USART1 :  TX - PA9
 *                 RX - PB7
 *       USART6 :  TX - PC6
 *                 RX - PC7
 *       USART10 : TX - PE3
 *                 RX - PE2
 *
 *       마지막으로 NVIC Setting에서 USART1번의 Global Interrupt 옵션 체크를 했는지 반드시 확인합니다.
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
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define RX_BUF_SIZE 64 // 수신 할 메시지의 최대 길이 (실제로는 63개의 char)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static uint8_t          debug_uart_rx_char;              // uart 수신 : 한 바이트의 문자
static char             debug_uart_rx_line[RX_BUF_SIZE]; // 한 바이트가 모여 이루어진 문자열
static volatile uint8_t debug_uart_rx_idx   = 0;         // rx_line의 배열 index
static volatile bool    debug_uart_rx_ready = false;     // 수신중 겹치지 않게 하는 flag

extern UART_HandleTypeDef huart1;                        // [STLink] uart 핸들러
extern UART_HandleTypeDef huart6;                        // uart6 핸들러
extern UART_HandleTypeDef huart10;                       // uart10 핸들러
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
 * @brief Uart 수신이 완료되어, 인터럽트 발생시 호출되는 Callback 함수
 *        이 함수내에서 하는 일은
 *        간단한 배열 조작에 불과하지만,
 *        이 함수 호출은 실제로는 인터럽트가 일어날때 호출된다는 점이 중요하다.
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (debug_uart_rx_ready) // 수신된 메시지가 처리가 되지않았을 경우, 수신을 받되, 버퍼에 쓰진 않는다.
  {
    HAL_UART_Receive_IT(&huart1, &debug_uart_rx_char, 1);
    return;
  }

  if (huart == &huart1)                                                             // 1. 해당 포트의 인터럽트가 맞다면
  {
    HAL_UART_Transmit(&huart1, &debug_uart_rx_char, sizeof(debug_uart_rx_char), 1); // 사용자가 입력한 글자를 그대로 출력 (사용환경을 위해)

    if (debug_uart_rx_char == '\n' || debug_uart_rx_char == '\r')                   // 4. 개행 문자가 들어왔다면,
    {
      debug_uart_rx_line[debug_uart_rx_idx] = 0;                                    // 5. 배열과 인덱스 초기화, 그리고 수신 준비가 완료됨을 표시한다.
      debug_uart_rx_idx                     = 0;
      debug_uart_rx_ready                   = true;                                 // 6. 이 boolean 변수를 가지고 main loop에서 활용한다.
    }
    else
    {
      if (debug_uart_rx_idx < RX_BUF_SIZE - 1)
      {
        debug_uart_rx_line[debug_uart_rx_idx++] = debug_uart_rx_char; // 2. debug_uart_rx_line 배열에 한글자씩 담는다.
      }
    }
    HAL_UART_Receive_IT(&huart1, &debug_uart_rx_char, 1);             // 3. 다음 글자를 수신한다.
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
  /* USER CODE BEGIN 2 */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET); // 처음에 LED를 꺼준다.
  HAL_UART_Receive_IT(&huart1, &debug_uart_rx_char, 1); // 인터럽트로 uart 수신 (1바이트 한글자)을 예약한다.
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  // 인터럽트가 일어나면, 콜백 함수에서 데이터를 저장 하고,
  // 로직은 이곳에서 처리한다.
  while (1)
  {
    if (debug_uart_rx_ready)                                       // 데이터 수신 완료 후 대기중
    {
      debug_uart_rx_ready = false;

      if (strcmp(debug_uart_rx_line, "LED ON") == 0)               // 들어온 문자열이 "LED ON\n"일때
      {
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET); // LED를 켜준다.
        char msg[] = "current : LED ON\n";
        HAL_UART_Transmit(&huart1, (uint8_t *)msg, sizeof(msg), 100);
      }
      else if (strcmp(debug_uart_rx_line, "LED OFF") == 0)         // 들어온 문자열이 "LED OFF\n"일때
      {
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);   // LED를 꺼준다.
        char msg[] = "current : LED OFF\n";
        HAL_UART_Transmit(&huart1, (uint8_t *)msg, sizeof(msg), 100);
      }
      else if (strcmp(debug_uart_rx_line, "UART6 SEND") == 0)      // 들어온 문자열이 "UART6 SEND\n"일때
      {
        char msg[] = "[SENT]Hello Uart!\n";
        HAL_UART_Transmit(&huart6, (uint8_t *)msg, sizeof(msg), 100);
      }
      else if (strcmp(debug_uart_rx_line, "UART10 SEND") == 0)     // 들어온 문자열이 "UART10 SEND\n"일때
      {
        char msg[] = "[SENT]Hello Uart!\n";
        HAL_UART_Transmit(&huart10, (uint8_t *)msg, sizeof(msg), 100);
      }
      else                                                         // 모르는 문자열일 경우
      {
        char msg[] = "[ERROR]unknown command\n";
        HAL_UART_Transmit(&huart1, (uint8_t *)msg, sizeof(msg), 100);
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
