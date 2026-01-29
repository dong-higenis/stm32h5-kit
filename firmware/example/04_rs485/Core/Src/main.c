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
 *        이전 예제 UART의 확장버전으로 구현하려고 합니다.
 *
 *        이번 예제는 RS485이며, UART, RS232와 크게 다르진 않습니다만
 *        통신 방향 제어, 버스 공유라는 개념의 독특한 차이점이 있습니다.
 *   
 *        이전 예제처럼, 명령어를 입력하여 상대방이 RS485메시지를 잘 받는지 확인해 보도록 합시다.
 *
 * @details
 *  - RS485는 Half-Duplex 방식입니다. (A핀과 B핀은 차동전압으로 송신)
 *    기존 UART 통신의 경우, TX/RX 두핀이 있었는데, RS485는 선이 하나만 있다고 생각하면서 구현하면, 수월할때가 많은 것 같습니다.
 *    송신(TX) 시와 수신(RX) 시
 *    데이터 방향을 GPIO로 직접 제어해야 합니다. (DE 핀)
 *    (쉽게 비유하자면, 지금은 받기만 한다. 지금은 보내기만 한다. 고정적으로 명시해두고 쓴다는 것입니다.)
 *
 *  - 여러 노드가 하나의 버스를 공유하므로,
 *    동시에 송신하면 충돌이 발생할 수 있습니다.
 *    따라서 송신 타이밍 제어와 응답 대기 구조가 중요합니다.
 *
 *  - 본 예제에서는 송신 전 DE 핀을 활성화하고,
 *    전송 완료 후 다시 수신 모드로 복귀하는 기본 흐름을 구현합니다.
 *
 *  - 수신은 인터럽트 기반으로 처리하여,
 *    메인 루프에서는 수신 완료 여부만 확인하고
 *    실제 명령 파싱과 동작은 메인 루프에서 수행합니다.
 *
 * @note
 *  - RS485 송신시 DE핀을 잠시 HIGH로 올려주고,
 *    송신이 확실히 끝났다면 LOW로 내려주어, 안정성을 확보해줍시다.
 *
 *  - RS485는 물리적으로는 UART이지만,
 *    버스 제어 로직이 추가되므로 소프트웨어 구조가 더 중요합니다.
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "gpdma.h"
#include "icache.h"
#include "usart.h"
#include "gpio.h"

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
#define RX_BUF_SIZE 64
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static uint8_t          debug_uart_rx_char;
static char             debug_uart_rx_line[RX_BUF_SIZE];
static volatile uint8_t debug_uart_rx_idx   = 0;
static volatile bool    debug_uart_rx_ready = false;

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart4; // RS485 핸들러 추가
extern UART_HandleTypeDef huart5; // RS485 핸들러 추가
extern UART_HandleTypeDef huart6;
extern UART_HandleTypeDef huart7;
extern UART_HandleTypeDef huart10;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (debug_uart_rx_ready)
  {
    HAL_UART_Receive_IT(&huart1, &debug_uart_rx_char, 1);
    return;
  }

  if (huart == &huart1)
  {
    HAL_UART_Transmit(&huart1, &debug_uart_rx_char, sizeof(debug_uart_rx_char), 1);

    if (debug_uart_rx_char == '\n' || debug_uart_rx_char == '\r')
    {
      debug_uart_rx_line[debug_uart_rx_idx] = 0;
      debug_uart_rx_idx                     = 0;
      debug_uart_rx_ready                   = true;
    }
    else
    {
      if (debug_uart_rx_idx < RX_BUF_SIZE - 1)
      {
        debug_uart_rx_line[debug_uart_rx_idx++] = debug_uart_rx_char;
      }
    }
    HAL_UART_Receive_IT(&huart1, &debug_uart_rx_char, 1);
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

  HAL_GPIO_WritePin(UART4_DE_GPIO_Port, UART4_DE_Pin, GPIO_PIN_RESET); // rs485 수신모드
  HAL_GPIO_WritePin(UART5_DE_GPIO_Port, UART5_DE_Pin, GPIO_PIN_RESET); // rs485 수신모드
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {
    if (debug_uart_rx_ready)
    {
      debug_uart_rx_ready = false;

      if (strcmp(debug_uart_rx_line, "LED ON") == 0)
      {
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
        char msg[] = "current : LED ON\n";
        HAL_UART_Transmit(&huart1, (uint8_t *)msg, sizeof(msg), 100);
      }
      else if (strcmp(debug_uart_rx_line, "LED OFF") == 0)
      {
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
        char msg[] = "current : LED OFF\n";
        HAL_UART_Transmit(&huart1, (uint8_t *)msg, sizeof(msg), 100);
      }
      else if (strcmp(debug_uart_rx_line, "UART6 SEND") == 0)
      {
        char msg[] = "[SENT]Hello Uart!\n";
        HAL_UART_Transmit(&huart6, (uint8_t *)msg, sizeof(msg), 100);
      }
      else if (strcmp(debug_uart_rx_line, "UART10 SEND") == 0)
      {
        char msg[] = "[SENT]Hello Uart!\n";
        HAL_UART_Transmit(&huart10, (uint8_t *)msg, sizeof(msg), 100);
      }
      else if (strcmp(debug_uart_rx_line, "RS232") == 0)
      {
        char msg[] = "[RS232]Hello Uart!\n";
        HAL_UART_Transmit(&huart7, (uint8_t *)msg, sizeof(msg), 100);
      }
      else if (strcmp(debug_uart_rx_line, "RS485 1") == 0)                // rs485 명령 추가!
      {
        HAL_GPIO_WritePin(UART5_DE_GPIO_Port, UART5_DE_Pin, GPIO_PIN_SET);// rs485 송신 모드

        char msg[] = "[RS485-1]Hello Uart!\n";
        HAL_UART_Transmit(&huart5, (uint8_t *)msg, sizeof(msg), 100);     // 보낸 후

        while (__HAL_UART_GET_FLAG(&huart5, UART_FLAG_TC) == RESET);      // 송신완료 플래그 (레지스터 플래그:TC)를 확인 , LOW이면 송신 끝

        HAL_GPIO_WritePin(UART5_DE_GPIO_Port, UART5_DE_Pin, GPIO_PIN_RESET);    // 수신모드
      }
      else if (strcmp(debug_uart_rx_line, "RS485 2") == 0)                // rs485 명령 추가!
      {
        HAL_GPIO_WritePin(UART4_DE_GPIO_Port, UART4_DE_Pin, GPIO_PIN_SET);// rs485 송신 모드

        char msg[] = "[RS485-2]Hello Uart!\n";
        HAL_UART_Transmit(&huart4, (uint8_t *)msg, sizeof(msg), 100);     // 보낸 후

        while (__HAL_UART_GET_FLAG(&huart4, UART_FLAG_TC) == RESET);      // 송신완료 플래그 (레지스터 플래그:TC)를 확인 , LOW이면 송신 끝

        HAL_GPIO_WritePin(UART4_DE_GPIO_Port, UART4_DE_Pin, GPIO_PIN_RESET);    // 수신모드
      }
      else
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

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV2;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_PCLK3;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
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
