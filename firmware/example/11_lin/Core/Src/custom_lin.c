#include "custom_lin.h"
#include <string.h>

/**
 * @brief LIN??
 *        - LIN(Local Interconnect Network) 통신은 자동차 내부의 저속 통신에 사용되는 프로토콜입니다.
 *        - LIN 프레임은 항상 Break 신호로 시작하며, 이는 버스 상의 모든 슬레이브 노드에게
 *        "새로운 프레임이 시작된다"는 것을 알리는 동기화 신호입니다.
 * 
 *         === LIN 프레임 구조 ===
 *        [Break] [Sync] [ID] [Data 0~8 bytes] [Checksum]
 * 
 *        === Break 신호 특징 ===
 *        - 최소 13비트 이상의 연속된 Dominant(Low) 신호
 *        - 일반 데이터(8비트 + 1 start + 1 stop = 10비트)보다 길어야 함
 *        - 슬레이브는 이 긴 Low 신호를 감지하여 프레임 시작을 인식
 *        - HAL_LIN_SendBreak() 함수를 사용 하겠습니다.
 * 
 *        (참고 : Sync는 0x55)
 *        (0x55 = 010101이라서, 수신 장치 측에서 명확한 비트길이를 인지 할 수 있게 됨)
 *        
 *        (참고 : ID)
 *        (ID는 CAN에서 CAN_ID와 비슷, 하위 6비트가 실제 ID고 나머지 2비트가 오류 검출용 패리티 비트로 쓰임)
 * 
 *        위 사실을 인지하고, 함수를 구현합시다.
 */

static uint8_t linCalcChecksum(uint8_t pid, uint8_t *data, uint8_t len);
static void linSendFrame(uint8_t ch, uint8_t pid, uint8_t *data, uint8_t len);
static void linReceiveMonitor(uint8_t ch, uint32_t timeout_ms);

#if _USE_LIN_MODE // LIN 사용시
#define MAX_CH 3
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart4; 
#endif

#if _USE_RS485_MODE // RS485 사용시
#ifndef MAX_CH
#define MAX_CH 1
#endif
extern UART_HandleTypeDef huart4; 
#endif

typedef struct 
{
  UART_HandleTypeDef *uart_handler; // 각 채널에 대한 핸들러
  USART_TypeDef      *instance;     // 인스턴스
} uart_t;

#if _USE_LIN_MODE // LIN 사용시 아래 항목 초기화
uart_t uart_list[MAX_CH] = 
{
  {&huart2, USART2},  
  {&huart3, USART3},
  {&huart4, UART4},
};
#endif

#if _USE_RS485_MODE // RS485 사용시 아래 항목 초기화
uart_t uart_list[MAX_CH] = 
{
  {&huart4, UART4},
};
#endif

void customLinInit(void)
{
  for (int i = 0; i < MAX_CH; i++)
  {
    // Instance 설정
    uart_list[i].uart_handler->Instance = uart_list[i].instance;
    
    // BaudRate 설정 
    #if _USE_LIN_MODE
    uart_list[i].uart_handler->Init.BaudRate = 9600; 
    #else  
    uart_list[i].uart_handler->Init.BaudRate = 115200;
    #endif
    
    uart_list[i].uart_handler->Init.WordLength = UART_WORDLENGTH_8B;
    uart_list[i].uart_handler->Init.StopBits = UART_STOPBITS_1;
    uart_list[i].uart_handler->Init.Parity = UART_PARITY_NONE;
    uart_list[i].uart_handler->Init.Mode = UART_MODE_TX_RX;
    uart_list[i].uart_handler->Init.HwFlowCtl = UART_HWCONTROL_NONE;
    uart_list[i].uart_handler->Init.OverSampling = UART_OVERSAMPLING_16;
    uart_list[i].uart_handler->Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    uart_list[i].uart_handler->Init.ClockPrescaler = UART_PRESCALER_DIV1;
    uart_list[i].uart_handler->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

    // uart_handler 포인터 전달
    if (HAL_UART_Init(uart_list[i].uart_handler) != HAL_OK)
    {
      Error_Handler();
    }
    if (HAL_UARTEx_SetTxFifoThreshold(uart_list[i].uart_handler, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
      Error_Handler();
    }
    if (HAL_UARTEx_SetRxFifoThreshold(uart_list[i].uart_handler, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
      Error_Handler();
    }
    if (HAL_UARTEx_DisableFifoMode(uart_list[i].uart_handler) != HAL_OK)
    {
      Error_Handler();
    }
  }  
}

void cliLin(cli_args_t *args)
{
  bool ret = false;
  
  // lin tx [ch] [pid] [data0] [data1] ...
  if (args->argc == 1 && args->isStr(0, "info"))
  {
    #ifdef _USE_LIN_MODE

    cliPrintf("LIN 준비 완료\n");
    cliPrintf("LIN CH [0-2]\n");

    #else

    cliPrintf("다른 cli 함수를 사용해 주십시오.\n");

    #endif
    ret = true;
  }
  else if (args->argc >= 3 && args->isStr(0, "tx"))
  {
    uint8_t ch = (uint8_t)args->getData(1);
    uint8_t pid = (uint8_t)args->getData(2);
    uint8_t data[8] = {0};
    uint8_t len = 0;

    // 채널 범위 체크
    if (ch >= MAX_CH)
    {
      cliPrintf("Error: 채널 범위는 0~%d\n", MAX_CH - 1);
      ret = true;
    }
    else
    {
      #ifdef _USE_LIN_MODE  

      // 데이터 파싱 (3번째 인자부터)
      for (int i = 3; i < args->argc && len < 8; i++)
      {
        data[len++] = (uint8_t)args->getData(i);
      }
      
      // 전송 정보 출력
      cliPrintf("LIN TX ch%d: PID=0x%02X, Len=%d\n", ch, pid, len);
      cliPrintf("  Data: ");
      for (int i = 0; i < len; i++)
      {
        cliPrintf("0x%02X ", data[i]);
      }
      
      uint8_t checksum = linCalcChecksum(pid, data, len);
      cliPrintf("\n  Checksum: 0x%02X\n", checksum);
      
      // LIN 프레임 전송
      linSendFrame(ch, pid, data, len);
      cliPrintf("  -> Sent!\n");
      
      #else

      cliPrintf("LIN 모드만 사용 가능한 CLI 함수 입니다.\n");

      #endif
    
      ret = true;
    }
  }
  else if (args->argc == 2 && args->isStr(0, "rx"))
  {
    uint8_t ch = (uint8_t)args->getData(1);
    
    if (ch >= MAX_CH)
    {
      cliPrintf("Error: 채널 범위 [0~%d]\n", MAX_CH - 1);
      ret = true;
    }
    else
    {
      #ifdef _USE_LIN_MODE

      linReceiveMonitor(ch, 0);

      #else

      cliPrintf("LIN 모드만 사용 가능한 CLI 함수 입니다.\n");

      #endif

      ret = true;
    }
  }
  
  // 도움말
  if (!ret)
  {
	cliPrintf("lin info\n");
    cliPrintf("lin tx [ch] [pid] [data0] [data1] ...\n");
    cliPrintf("lin rx [ch]\n");
  }
}


/**
 * @brief LIN 체크섬 계산 (Classic Checksum)
 */
static uint8_t linCalcChecksum(uint8_t pid, uint8_t *data, uint8_t len)
{
  uint16_t sum = pid;
  
  for (int i = 0; i < len; i++)
  {
    sum += data[i];
    if (sum > 0xFF)  // Carry 처리
    {
      sum = (sum & 0xFF) + 1;
    }
  }
  
  return (uint8_t)(~sum);  // 1의 보수
}

/**
 * @brief LIN 프레임 전송
 * @param ch 채널 (0~2)
 * @param pid Protected ID
 * @param data 데이터 배열
 * @param len 데이터 길이 (1~8)
 */
static void linSendFrame(uint8_t ch, uint8_t pid, uint8_t *data, uint8_t len)
{
  if (ch >= MAX_CH || len > 8) 
  {
    return;
  } 
  
  uint8_t frame[11];  // Sync(1) + PID(1) + Data(8) + Checksum(1)
  uint8_t idx = 0;
  
  // 1. Break 전송 (UART 핸들러 전달)
  HAL_LIN_SendBreak(uart_list[ch].uart_handler);
  HAL_Delay(1);
  
  // 2. Sync Byte
  frame[idx++] = 0x55;
  
  // 3. PID
  frame[idx++] = pid;
  
  // 4. Data
  for (int i = 0; i < len; i++)
  {
    frame[idx++] = data[i];
  }
  
  // 5. Checksum
  frame[idx++] = linCalcChecksum(pid, data, len);
  
  // 전송
  HAL_UART_Transmit(uart_list[ch].uart_handler, frame, idx, 100);
}

/**
 * @brief LIN 프레임 수신 및 파싱
 * @param ch 채널 (0~2)
 * @param timeout_ms 타임아웃 (밀리초)
 */
static void linReceiveMonitor(uint8_t ch, uint32_t timeout_ms)
{

  enum
  {
    WAIT_BREAK,
    WAIT_SYNC,
    GOT_PID,
    GOT_DATA,
  };

  if (ch >= MAX_CH) 
  {
    return; 
  }
  
  cliPrintf("LIN RX Monitor ch%d @ 9600bps\n", ch);
  cliPrintf("Press 'q' to quit\n\n");
  
  uint8_t frame_state = WAIT_BREAK; 
  uint8_t pid = 0;
  uint8_t data[8] = {0};
  uint8_t data_idx = 0;
  uint32_t pre_time = 0;
  
  while(1)
  {
    // UART 수신 가능 여부 확인
    if (__HAL_UART_GET_FLAG(uart_list[ch].uart_handler, UART_FLAG_RXNE))
    {
      uint8_t rx_data = (uint8_t)(uart_list[ch].uart_handler->Instance->RDR & 0xFF);
      uint32_t current_time = HAL_GetTick();
      
      // 타임아웃: 프레임 리셋 (100ms)
      if (frame_state > 0 && (current_time - pre_time > 100))
      {
        cliPrintf("\n[Timeout]\n\n");
        frame_state = WAIT_BREAK;
        data_idx = 0;
      }
      pre_time = current_time;
      
      switch (frame_state)
      {
        case WAIT_BREAK:  // Break 대기 (0x00 감지)
          if (rx_data == 0x00)
          {
            cliPrintf("[BREAK] ");
            frame_state = WAIT_SYNC;
          }
          break;
          
        case WAIT_SYNC:  // Sync 대기 (0x55)
          if (rx_data == 0x55)
          {
            cliPrintf("[SYNC] ");
            frame_state = GOT_PID;
          }
          else
          {
            cliPrintf("[ERR: 기대한 SYNC = 0x55, 실제로 받은 SYNC = 0x%02X]\n\n", rx_data);
            frame_state = WAIT_BREAK;
          }
          break;
          
        case GOT_PID:  // PID 수신
          pid = rx_data;
          cliPrintf("[PID:0x%02X] ", pid);
          frame_state = GOT_DATA;
          data_idx = 0;
          break;
          
        case GOT_DATA:  // Data 수신
          data[data_idx++] = rx_data;
          cliPrintf("0x%02X ", rx_data);
          
          // 최대 9바이트 (Data 8 + Checksum 1) 수신 시 완료
          if (data_idx >= 9)
          {
            uint8_t received_checksum = data[data_idx - 1];
            uint8_t data_len = data_idx - 1;
            
            // 체크섬 검증
            uint8_t calc_checksum = linCalcChecksum(pid, data, data_len);
            
            cliPrintf("\n  [Checksum: Received=0x%02X, Calculated=0x%02X", 
                      received_checksum, calc_checksum);
            
            if (received_checksum == calc_checksum)
            {
              cliPrintf(" OK]\n");
            }
            else
            {
              cliPrintf(" FAIL]\n");
            }
            
            cliPrintf("\n");
            frame_state = WAIT_BREAK;
            data_idx = 0;
          }
          break;
      }
    }
    
    // 사용자 입력 체크 (종료)
    if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE))
    {
      uint8_t key = (uint8_t)(huart1.Instance->RDR & 0xFF);
      if (key == 'q' || key == 'Q')
      {
        break;
      }
    }
    
    HAL_Delay(1);
  }
  
  cliPrintf("\nLIN RX 모니터 종료.\n");
}
