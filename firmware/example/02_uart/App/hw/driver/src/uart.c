#include "uart.h"
#include "gpio.h"
#include "qbuffer.h"
#include "util.h"


#ifdef _USE_HW_UART


#define UART_RX_BUF_LENGTH 1024
#define MAX_BUF_SIZE       100

/**
 * @brief UART 한 채널의 상태 구조체
 */
typedef struct
{
  bool     is_open;              // 이 UART가 열려 있는지
  uint32_t baud;                 // 현재 설정된 통신 속도

  uint8_t   rx_buf[1024];        // 수신 데이터 임시 저장 버퍼
  qbuffer_t qbuffer;             // rx_buf를 감싸는 링버퍼

  UART_HandleTypeDef *p_huart;   // HAL UART 핸들
  DMA_HandleTypeDef  *p_hdma_rx; // DMA 핸들 (RX용)

  uint32_t rx_cnt;               // 지금까지 받은 바이트 수
  uint32_t tx_cnt;               // 지금까지 보낸 바이트 수
} uart_tbl_t;

/**
 * @brief UART TYPE
 */
typedef enum
{
  UART_TYPE_NORMAL,
} uart_type_t;

/**
 * @brief 하드웨어 매핑 테이블
 */
typedef struct
{
  const char         *p_msg;     // 이름 (CLI 출력용)
  USART_TypeDef      *p_uart;    // USART 레지스터
  UART_HandleTypeDef *p_huart;   // HAL 핸들
  DMA_HandleTypeDef  *p_hdma_rx; // RX DMA
  DMA_HandleTypeDef  *p_hdma_tx; // TX DMA
  uart_type_t         uart_type; // NORMAL / RS485 / LIN 등
} uart_hw_t;


#ifdef _USE_HW_CLI
static void cliUart(cli_args_t *args);
#endif


static bool is_init = false;

__attribute__((section(".non_cache"))) static uart_tbl_t uart_tbl[UART_MAX_CH];

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart6;
extern UART_HandleTypeDef huart10;

extern DMA_HandleTypeDef handle_GPDMA1_Channel1;
extern DMA_HandleTypeDef handle_GPDMA1_Channel2;

const static uart_hw_t uart_hw_tbl[UART_MAX_CH] =
{
  {"Debug UART   ", USART1,  &huart1,  &handle_GPDMA1_Channel1, NULL, UART_TYPE_NORMAL},
  {"UART_1       ", USART6,  &huart6,  &handle_GPDMA1_Channel2, NULL, UART_TYPE_NORMAL},
  {"UART_2       ", USART10, &huart10, &handle_GPDMA1_Channel2, NULL, UART_TYPE_NORMAL},
};

/**
 *
 */
bool uartInit(void)
{
  for (int i = 0; i < UART_MAX_CH; i++)
  {
    uart_tbl[i].is_open = false;

    switch (i)
    {
      case HW_UART_CH_DEBUG:
      case HW_UART_CH_UART_1:
      case HW_UART_CH_UART_2:
        uart_tbl[i].baud = 115200;
        break;
    }

    uart_tbl[i].rx_cnt = 0;
    uart_tbl[i].tx_cnt = 0;
  }

  is_init = true;

#ifdef _USE_HW_CLI
  cliAdd("uart", cliUart); // CLI 명령 등록 (example 03)
#endif

  return true;
}

bool uartDeInit(void)
{
  return true;
}

bool uartIsInit(void)
{
  return is_init;
}

bool uartOpen(uint8_t ch, uint32_t baud)
{
  bool              ret = false;       // 함수 성공 여부
  HAL_StatusTypeDef ret_hal;           // HAL 함수 결과 저장용

  // 채널 번호가 범위를 벗어나면 실패
  if (ch >= UART_MAX_CH) return false;

  // 이미 열려 있고, 같은 baud라면 재초기화할 필요 없음
  if (uart_tbl[ch].is_open == true && uart_tbl[ch].baud == baud)
  {
    return true;
  }

  // 채널 번호에 따라 실제 UART 하드웨어를 설정
  switch (ch)
  {
    case HW_UART_CH_DEBUG:
    case HW_UART_CH_UART_1:
    case HW_UART_CH_UART_2:

      // 현재 채널의 baudrate 저장
      uart_tbl[ch].baud = baud;

      // 이 채널이 사용할 HAL UART 핸들 포인터 연결
      uart_tbl[ch].p_huart = uart_hw_tbl[ch].p_huart;

      // 이 채널이 사용할 RX DMA 핸들 연결
      uart_tbl[ch].p_hdma_rx = uart_hw_tbl[ch].p_hdma_rx;

      // HAL 핸들에 실제 UART 레지스터(USARTx) 연결
      uart_tbl[ch].p_huart->Instance = uart_hw_tbl[ch].p_uart;

      // ---- UART 기본 통신 설정 ----
      uart_tbl[ch].p_huart->Init.BaudRate   = baud;                  // 통신 속도
      uart_tbl[ch].p_huart->Init.WordLength = UART_WORDLENGTH_8B;    // 데이터 비트: 8bit
      uart_tbl[ch].p_huart->Init.StopBits   = UART_STOPBITS_1;       // 스톱비트: 1
      uart_tbl[ch].p_huart->Init.Parity     = UART_PARITY_NONE;      // 패리티 없음
      uart_tbl[ch].p_huart->Init.Mode       = UART_MODE_TX_RX;       // 송신 + 수신
      uart_tbl[ch].p_huart->Init.HwFlowCtl  = UART_HWCONTROL_NONE;   // RTS/CTS 사용 안 함

      // 샘플링/클럭 관련 세부 옵션
      uart_tbl[ch].p_huart->Init.OverSampling   = UART_OVERSAMPLING_16;
      uart_tbl[ch].p_huart->Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
      uart_tbl[ch].p_huart->Init.ClockPrescaler = UART_PRESCALER_DIV1;

      // 고급 기능(반전, 자동보드레이트 등) 사용 안 함
      uart_tbl[ch].p_huart->AdvancedInit.AdvFeatureInit =
          UART_ADVFEATURE_NO_INIT;

      // 수신 데이터 저장용 링버퍼 생성
      qbufferCreate(&uart_tbl[ch].qbuffer,
                    &uart_tbl[ch].rx_buf[0],
                    UART_RX_BUF_LENGTH);

      // DMA 컨트롤러 클럭 활성화 (UART RX DMA 사용을 위해)
      __HAL_RCC_GPDMA1_CLK_ENABLE();

      // 혹시 이전 설정이 남아있으면 초기화
      HAL_UART_DeInit(uart_tbl[ch].p_huart);

      // HAL을 통해 UART 초기화
      ret_hal = HAL_UART_Init(uart_tbl[ch].p_huart);

      // 초기화 성공 시
      if (ret_hal == HAL_OK)
      {
        ret = true;                     // 성공 표시
        uart_tbl[ch].is_open = true;    // 채널 열림 표시

        // 링버퍼를 다시 초기화 (안전용)
        qbufferCreate(&uart_tbl[ch].qbuffer,
                      &uart_tbl[ch].rx_buf[0],
                      UART_RX_BUF_LENGTH);

        // 링버퍼 인덱스 초기화
        uart_tbl[ch].qbuffer.in  = 0;
        uart_tbl[ch].qbuffer.out = 0;
      }
      break;
  }

  // 성공 여부 반환
  return ret;
}


bool uartClose(uint8_t ch)
{
  if (ch >= UART_MAX_CH) return false;

  uart_tbl[ch].is_open = false;

  return true;
}

uint32_t uartAvailable(uint8_t ch)
{
  uint32_t ret = 0;

  switch (ch)
  {
    case HW_UART_CH_DEBUG:
    case HW_UART_CH_UART_1:
    case HW_UART_CH_UART_2:
      {
        // Overrun 에러 클리어 (수신 재개)
        if (__HAL_UART_GET_FLAG(uart_tbl[ch].p_huart, UART_FLAG_ORE))
        {
          __HAL_UART_CLEAR_FLAG(uart_tbl[ch].p_huart, UART_CLEAR_OREF);
        }

        // Framing 에러 클리어
        if (__HAL_UART_GET_FLAG(uart_tbl[ch].p_huart, UART_FLAG_FE))
        {
          __HAL_UART_CLEAR_FLAG(uart_tbl[ch].p_huart, UART_CLEAR_FEF);
        }

        // 하드웨어 RX 레지스터에 데이터 있으면
        // qbuffer로 하나씩 옮긴다.
        while (__HAL_UART_GET_FLAG(uart_tbl[ch].p_huart, UART_FLAG_RXNE))
        {
          uint8_t data = uart_tbl[ch].p_huart->Instance->RDR;
          qbufferWrite(&uart_tbl[ch].qbuffer, &data, 1);
          uart_tbl[ch].rx_cnt++;
        }

        ret = qbufferAvailable(&uart_tbl[ch].qbuffer);
      }
      break;
  }

  return ret; // 데이터 잔존 개수 반환
}

bool uartIsOpen(uint8_t ch)
{
  bool ret = false;

  switch (ch)
  {
    case HW_UART_CH_DEBUG:
    case HW_UART_CH_UART_1:
    case HW_UART_CH_UART_2:
      ret = uart_tbl[ch].is_open = true;
      break;
  }
  return ret;
}

bool uartFlush(uint8_t ch)
{
  uint32_t pre_time;

  pre_time = millis();

  while (uartAvailable(ch))
  {
    if (millis() - pre_time >= 10)
    {
      break;
    }
    uartRead(ch);
  }

  return true;
}

uint8_t uartRead(uint8_t ch)
{
  uint8_t ret = 0;

  switch (ch)
  {
    case HW_UART_CH_DEBUG:
    case HW_UART_CH_UART_1:
    case HW_UART_CH_UART_2:
      qbufferRead(&uart_tbl[ch].qbuffer, &ret, 1);
      break;
  }
  // rx_cnt는 uartAvailable()에서 이미 증가시켰으므로 여기서는 제거

  return ret;
}

uint32_t uartPrintf(uint8_t ch, const char *fmt, ...)
{
  char     buf[256];
  va_list  args;
  int      len;
  uint32_t ret;

  va_start(args, fmt);
  len = vsnprintf(buf, 256, fmt, args);

  ret = uartWrite(ch, (uint8_t *)buf, len);

  va_end(args);


  return ret;
}

uint32_t uartGetBaud(uint8_t ch)
{
  uint32_t ret = 0;


  if (ch >= UART_MAX_CH)
    return 0;
  ret = uart_tbl[ch].baud;

  return ret;
}

uint32_t uartGetRxCnt(uint8_t ch)
{
  if (ch >= UART_MAX_CH) return 0;

  return uart_tbl[ch].rx_cnt;
}

uint32_t uartGetTxCnt(uint8_t ch)
{
  if (ch >= UART_MAX_CH) return 0;

  return uart_tbl[ch].tx_cnt;
}


#endif
