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
 *        RS232는 UART처럼 사용하시면 되기 때문에,
 *        STLink 포트를 이용하여, "RS232" 라고 입력 후 엔터를 치고,
 *        상대편이 rs232 메시지를 잘 받는지 관찰해 봅시다.
 * 
 * @note  ioc에서 baudrate는 115200으로 맞춰줍시다.
 *        uart7을 사용하며, CTS/RTS 옵션을 체크해 줍시다. (해당 옵션은 하드웨어적으로 상대편에게 REQUEST및 RESPONSE를 해줍니다.)
 *        따라서 안정적인 상황에서만 통신을 지속 할 수 있게 해줍니다.
 *        현재 예제 수준에서는 불필요할지도 모르지만, 학습을 위해서라면 체크해두는게 좋겠습니다.
 *        핀번호는 아래와 같습니다.
 *        
 *        RX - PE7
 *        TX - PE8
 *        RTS - PE9
 *        CTS - PE10 
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
extern UART_HandleTypeDef huart6;
extern UART_HandleTypeDef huart7; // RS232 핸들러                        
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
  /* USER CODE BEGIN 2 */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET); 
  HAL_UART_Receive_IT(&huart1, &debug_uart_rx_char, 1); 
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
      else if (strcmp(debug_uart_rx_line, "RS232") == 0) // 해당 줄과 같이 rs232 명령도 추가!
      {
        char msg[] = "[RS232]Hello Uart!\n";
        HAL_UART_Transmit(&huart7, (uint8_t *)msg, sizeof(msg), 100);
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
