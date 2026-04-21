#include "uart.h"


#ifdef _USE_HW_UART
#include "cli.h"
#include "gpio.h"
#include "qbuffer.h"
#if HW_USB_CDC == 1
#include "cdc.h"
#endif

#define UART_RX_BUF_LENGTH 1024

typedef struct
{
  bool     is_open;
  uint32_t baud;

  uint8_t             rx_buf[UART_RX_BUF_LENGTH];
  qbuffer_t           qbuffer;
  UART_HandleTypeDef *p_huart;
  DMA_HandleTypeDef  *p_hdma_rx;

  uint32_t rx_cnt;
  uint32_t tx_cnt;
} uart_tbl_t;

typedef enum
{
  UART_TYPE_NORMAL,
  UART_TYPE_RS485,
  UART_TYPE_LIN,
  UART_TYPE_RS232
} uart_type_t;

typedef struct
{
  const char         *p_msg;
  USART_TypeDef      *p_uart;
  UART_HandleTypeDef *p_huart;
  DMA_HandleTypeDef  *p_hdma_rx;
  uart_type_t         uart_type;
} uart_hw_t;


#if CLI_USE(HW_UART)
static void cliUart(cli_args_t *args);
#endif


static bool is_init = false;

__attribute__((section(".non_cache"))) static uart_tbl_t uart_tbl[UART_MAX_CH];


/**
 * @brief 핸들러
 */
static UART_HandleTypeDef huart1;
static UART_HandleTypeDef huart2;
static UART_HandleTypeDef huart3;
static UART_HandleTypeDef huart4;
static UART_HandleTypeDef huart5;
static UART_HandleTypeDef huart6;
static UART_HandleTypeDef huart7;
static UART_HandleTypeDef huart10;

/**
 * @brief DMA
 */
static DMA_HandleTypeDef handle_GPDMA1_Channel0;
static DMA_HandleTypeDef handle_GPDMA1_Channel1;
static DMA_HandleTypeDef handle_GPDMA1_Channel2;
static DMA_HandleTypeDef handle_GPDMA1_Channel3;
static DMA_HandleTypeDef handle_GPDMA1_Channel4;
static DMA_HandleTypeDef handle_GPDMA1_Channel5;
static DMA_HandleTypeDef handle_GPDMA2_Channel0;
static DMA_HandleTypeDef handle_GPDMA2_Channel1;

// Node
static DMA_NodeTypeDef Node_GPDMA1_Channel0;
static DMA_NodeTypeDef Node_GPDMA1_Channel1;
static DMA_NodeTypeDef Node_GPDMA1_Channel2;
static DMA_NodeTypeDef Node_GPDMA1_Channel3;
static DMA_NodeTypeDef Node_GPDMA1_Channel4;
static DMA_NodeTypeDef Node_GPDMA1_Channel5;
static DMA_NodeTypeDef Node_GPDMA2_Channel0;
static DMA_NodeTypeDef Node_GPDMA2_Channel1;

// Linked-List
static DMA_QListTypeDef List_GPDMA1_Channel0;
static DMA_QListTypeDef List_GPDMA1_Channel1;
static DMA_QListTypeDef List_GPDMA1_Channel2;
static DMA_QListTypeDef List_GPDMA1_Channel3;
static DMA_QListTypeDef List_GPDMA1_Channel4;
static DMA_QListTypeDef List_GPDMA1_Channel5;
static DMA_QListTypeDef List_GPDMA2_Channel0;
static DMA_QListTypeDef List_GPDMA2_Channel1;

const static uart_hw_t uart_hw_tbl[UART_MAX_CH] =
{
  // ch0: Debug
  {"Debug UART  ", USART1,  &huart1,  &handle_GPDMA1_Channel0, UART_TYPE_NORMAL},
  // ch1: UART_1
  {"UART_1      ", USART6,  &huart6,  &handle_GPDMA1_Channel1, UART_TYPE_NORMAL},
  // ch2: UART_2
  {"UART_2      ", USART10, &huart10, &handle_GPDMA1_Channel2, UART_TYPE_NORMAL},
  // ch3: RS232
  {"RS232       ", UART7,   &huart7,  &handle_GPDMA2_Channel1, UART_TYPE_RS232 },
  // ch4: RS485_1
  {"RS485_1     ", UART5,   &huart5,  &handle_GPDMA1_Channel3, UART_TYPE_RS485 },
  // ch5: RS485_2
  {"RS485_2     ", UART4,   &huart4,  &handle_GPDMA1_Channel4, UART_TYPE_RS485 },
  // ch6: LIN_1
  {"LIN_1       ", USART2,  &huart2,  &handle_GPDMA1_Channel5, UART_TYPE_LIN   },
  // ch7: LIN_2
  {"LIN_2       ", USART3,  &huart3,  &handle_GPDMA2_Channel0, UART_TYPE_LIN   },
  // ch8: LIN_3
  {"LIN_3       ", UART4,   &huart4,  &handle_GPDMA1_Channel4, UART_TYPE_LIN   },
};

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
      case HW_UART_CH_RS232:
      case HW_UART_CH_RS485_1:
      case HW_UART_CH_RS485_2:
        uart_tbl[i].baud = 115200;
        break;

      case HW_UART_CH_LIN_1:
      case HW_UART_CH_LIN_2:
      case HW_UART_CH_LIN_3:
        uart_tbl[i].baud = 9600;
        break;
    }
  }

  is_init = true;

#if CLI_USE(HW_UART)
  cliAdd("uart", cliUart);
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
  bool              ret = false;
  HAL_StatusTypeDef ret_hal;


  if (ch >= UART_MAX_CH)
    return false;

  if (uart_tbl[ch].is_open == true && uart_tbl[ch].baud == baud)
  {
    return true;
  }

  switch (ch)
  {
    default:
      uart_tbl[ch].baud = baud;

      uart_tbl[ch].p_huart           = uart_hw_tbl[ch].p_huart;
      uart_tbl[ch].p_huart->Instance = uart_hw_tbl[ch].p_uart;

      // uart_tbl[ch].p_hdma_rx = uart_hw_tbl[ch].p_hdma_rx;

      uart_tbl[ch].p_huart->Init.BaudRate               = baud;
      uart_tbl[ch].p_huart->Init.WordLength             = UART_WORDLENGTH_8B;
      uart_tbl[ch].p_huart->Init.StopBits               = UART_STOPBITS_1;
      uart_tbl[ch].p_huart->Init.Parity                 = UART_PARITY_NONE;
      uart_tbl[ch].p_huart->Init.Mode                   = UART_MODE_TX_RX;
      uart_tbl[ch].p_huart->Init.HwFlowCtl              = UART_HWCONTROL_NONE;
      uart_tbl[ch].p_huart->Init.OverSampling           = UART_OVERSAMPLING_16;
      uart_tbl[ch].p_huart->Init.OneBitSampling         = UART_ONE_BIT_SAMPLE_DISABLE;
      uart_tbl[ch].p_huart->Init.ClockPrescaler         = UART_PRESCALER_DIV1;
      uart_tbl[ch].p_huart->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

      qbufferCreate(&uart_tbl[ch].qbuffer, &uart_tbl[ch].rx_buf[0], UART_RX_BUF_LENGTH);

      __HAL_RCC_GPDMA1_CLK_ENABLE();
      __HAL_RCC_GPDMA2_CLK_ENABLE();


      HAL_UART_DeInit(uart_tbl[ch].p_huart);

      if (uart_hw_tbl[ch].uart_type == UART_TYPE_RS485)
      {
        ret_hal = HAL_RS485Ex_Init(uart_tbl[ch].p_huart, UART_DE_POLARITY_HIGH, 0, 0);
      }
      else
      {
        ret_hal = HAL_UART_Init(uart_tbl[ch].p_huart);
      }

      if (ch == HW_UART_CH_UART_2)
      {
        // 1). USART10 초기화 직후 핵심 레지스터 상태를 확인한다.
        logPrintf("[UART10] Init ret=%d err=0x%08lX ISR=0x%08lX CR1=0x%08lX CR2=0x%08lX CR3=0x%08lX BRR=0x%08lX\n",
                  ret_hal,
                  uart_tbl[ch].p_huart->ErrorCode,
                  uart_tbl[ch].p_huart->Instance->ISR,
                  uart_tbl[ch].p_huart->Instance->CR1,
                  uart_tbl[ch].p_huart->Instance->CR2,
                  uart_tbl[ch].p_huart->Instance->CR3,
                  uart_tbl[ch].p_huart->Instance->BRR);
      }

      if (ret_hal == HAL_OK)
      {
        if (uart_hw_tbl[ch].uart_type == UART_TYPE_LIN)
        {
          SET_BIT(uart_tbl[ch].p_huart->Instance->CR2, USART_CR2_LINEN);
          SET_BIT(uart_tbl[ch].p_huart->Instance->CR2, USART_CR2_LBDL); // 11-bit break detection
        }
      }


      if (ret_hal == HAL_OK)
      {
        ret                  = true;
        uart_tbl[ch].is_open = true;

        ret_hal = HAL_UART_Receive_DMA(uart_tbl[ch].p_huart, (uint8_t *)&uart_tbl[ch].rx_buf[0], UART_RX_BUF_LENGTH);

        if (ch == HW_UART_CH_UART_2)
        {
          // 2). DMA 시작 결과와 TX/RX 핀 입력 상태를 함께 확인한다.
          logPrintf("[UART10] DMA ret=%d err=0x%08lX PE2=%d PE3=%d UE=%d TE=%d RE=%d CBR1=%lu\n",
                    ret_hal,
                    uart_tbl[ch].p_huart->ErrorCode,
                    HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_2),
                    HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_3),
                    (uart_tbl[ch].p_huart->Instance->CR1 & USART_CR1_UE) ? 1 : 0,
                    (uart_tbl[ch].p_huart->Instance->CR1 & USART_CR1_TE) ? 1 : 0,
                    (uart_tbl[ch].p_huart->Instance->CR1 & USART_CR1_RE) ? 1 : 0,
                    (uart_tbl[ch].p_huart->hdmarx != NULL) ? uart_tbl[ch].p_huart->hdmarx->Instance->CBR1 : 0);
        }

        if (ret_hal != HAL_OK)
        {
          ret = false;
        }

        uart_tbl[ch].qbuffer.in  = uart_tbl[ch].qbuffer.len - uart_tbl[ch].p_huart->hdmarx->Instance->CBR1;
        uart_tbl[ch].qbuffer.out = uart_tbl[ch].qbuffer.in;
      }
      break;
  }

  return ret;
}

bool uartClose(uint8_t ch)
{
  if (ch >= UART_MAX_CH)
    return false;

  uart_tbl[ch].is_open = false;

  return true;
}

uint32_t uartAvailable(uint8_t ch)
{
  uint32_t ret = 0;


  switch (ch)
  {
    default:
      uart_tbl[ch].qbuffer.in = (uart_tbl[ch].qbuffer.len - uart_tbl[ch].p_huart->hdmarx->Instance->CBR1);
      ret                     = qbufferAvailable(&uart_tbl[ch].qbuffer);
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
    default:
      qbufferRead(&uart_tbl[ch].qbuffer, &ret, 1);
      break;
  }
  uart_tbl[ch].rx_cnt++;

  return ret;
}

uint32_t uartWrite(uint8_t ch, uint8_t *p_data, uint32_t length)
{
  uint32_t ret = 0;


  switch (ch)
  {
    default:
      if (HAL_UART_Transmit(uart_tbl[ch].p_huart, p_data, length, 100) == HAL_OK)
      {
        ret = length;
      }
      break;
  }
  uart_tbl[ch].tx_cnt += ret;

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

#if HW_USB_CDC == 1
  if (ch == HW_UART_CH_USB)
    ret = cdcGetBaud();
  else
    ret = uart_tbl[ch].baud;
#else
  ret = uart_tbl[ch].baud;
#endif

  return ret;
}

uint32_t uartGetRxCnt(uint8_t ch)
{
  if (ch >= UART_MAX_CH)
    return 0;

  return uart_tbl[ch].rx_cnt;
}

uint32_t uartGetTxCnt(uint8_t ch)
{
  if (ch >= UART_MAX_CH)
    return 0;

  return uart_tbl[ch].tx_cnt;
}

void HAL_UART_MspInit(UART_HandleTypeDef *uartHandle)
{
  GPIO_InitTypeDef         GPIO_InitStruct     = {0};
  DMA_NodeConfTypeDef      NodeConfig          = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  if (uartHandle->Instance == USART1)
  {
    /** Initializes the peripherals clock
     */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART1;
    PeriphClkInitStruct.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* USART1 clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PB7     ------> USART1_RX
    */
    GPIO_InitStruct.Pin       = GPIO_PIN_9;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin       = GPIO_PIN_7;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* USART1 DMA Init */
    /* GPDMA1_REQUEST_USART1_RX Init */
    NodeConfig.NodeType                         = DMA_GPDMA_LINEAR_NODE;
    NodeConfig.Init.Request                     = GPDMA1_REQUEST_USART1_RX;
    NodeConfig.Init.BlkHWRequest                = DMA_BREQ_SINGLE_BURST;
    NodeConfig.Init.Direction                   = DMA_PERIPH_TO_MEMORY;
    NodeConfig.Init.SrcInc                      = DMA_SINC_FIXED;
    NodeConfig.Init.DestInc                     = DMA_DINC_INCREMENTED;
    NodeConfig.Init.SrcDataWidth                = DMA_SRC_DATAWIDTH_BYTE;
    NodeConfig.Init.DestDataWidth               = DMA_DEST_DATAWIDTH_BYTE;
    NodeConfig.Init.SrcBurstLength              = 1;
    NodeConfig.Init.DestBurstLength             = 1;
    NodeConfig.Init.TransferAllocatedPort       = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT0;
    NodeConfig.Init.TransferEventMode           = DMA_TCEM_BLOCK_TRANSFER;
    NodeConfig.Init.Mode                        = DMA_NORMAL;
    NodeConfig.TriggerConfig.TriggerPolarity    = DMA_TRIG_POLARITY_MASKED;
    NodeConfig.DataHandlingConfig.DataExchange  = DMA_EXCHANGE_NONE;
    NodeConfig.DataHandlingConfig.DataAlignment = DMA_DATA_RIGHTALIGN_ZEROPADDED;
    if (HAL_DMAEx_List_BuildNode(&NodeConfig, &Node_GPDMA1_Channel0) != HAL_OK)
    {
      Error_Handler();
    }

    if (HAL_DMAEx_List_InsertNode(&List_GPDMA1_Channel0, NULL, &Node_GPDMA1_Channel0) != HAL_OK)
    {
      Error_Handler();
    }

    if (HAL_DMAEx_List_SetCircularMode(&List_GPDMA1_Channel0) != HAL_OK)
    {
      Error_Handler();
    }

    handle_GPDMA1_Channel0.Instance                         = GPDMA1_Channel0;
    handle_GPDMA1_Channel0.InitLinkedList.Priority          = DMA_LOW_PRIORITY_LOW_WEIGHT;
    handle_GPDMA1_Channel0.InitLinkedList.LinkStepMode      = DMA_LSM_FULL_EXECUTION;
    handle_GPDMA1_Channel0.InitLinkedList.LinkAllocatedPort = DMA_LINK_ALLOCATED_PORT0;
    handle_GPDMA1_Channel0.InitLinkedList.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
    handle_GPDMA1_Channel0.InitLinkedList.LinkedListMode    = DMA_LINKEDLIST_CIRCULAR;
    if (HAL_DMAEx_List_Init(&handle_GPDMA1_Channel0) != HAL_OK)
    {
      Error_Handler();
    }

    if (HAL_DMAEx_List_LinkQ(&handle_GPDMA1_Channel0, &List_GPDMA1_Channel0) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle, hdmarx, handle_GPDMA1_Channel0);

    if (HAL_DMA_ConfigChannelAttributes(&handle_GPDMA1_Channel0, DMA_CHANNEL_NPRIV) != HAL_OK)
    {
      Error_Handler();
    }

  }

  if (uartHandle->Instance == USART2)
  {
    /** Initializes the peripherals clock
     */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART2;
    PeriphClkInitStruct.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* USART2 clock enable */
    __HAL_RCC_USART2_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**USART2 GPIO Configuration
    PD5     ------> USART2_TX
    PD6     ------> USART2_RX
    */
    GPIO_InitStruct.Pin       = GPIO_PIN_5 | GPIO_PIN_6;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* USART2 DMA Init */
    /* GPDMA1_REQUEST_USART2_RX Init */
    NodeConfig.NodeType                         = DMA_GPDMA_LINEAR_NODE;
    NodeConfig.Init.Request                     = GPDMA1_REQUEST_USART2_RX;
    NodeConfig.Init.BlkHWRequest                = DMA_BREQ_SINGLE_BURST;
    NodeConfig.Init.Direction                   = DMA_PERIPH_TO_MEMORY;
    NodeConfig.Init.SrcInc                      = DMA_SINC_FIXED;
    NodeConfig.Init.DestInc                     = DMA_DINC_INCREMENTED;
    NodeConfig.Init.SrcDataWidth                = DMA_SRC_DATAWIDTH_BYTE;
    NodeConfig.Init.DestDataWidth               = DMA_DEST_DATAWIDTH_BYTE;
    NodeConfig.Init.SrcBurstLength              = 1;
    NodeConfig.Init.DestBurstLength             = 1;
    NodeConfig.Init.TransferAllocatedPort       = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT0;
    NodeConfig.Init.TransferEventMode           = DMA_TCEM_BLOCK_TRANSFER;
    NodeConfig.Init.Mode                        = DMA_NORMAL;
    NodeConfig.TriggerConfig.TriggerPolarity    = DMA_TRIG_POLARITY_MASKED;
    NodeConfig.DataHandlingConfig.DataExchange  = DMA_EXCHANGE_NONE;
    NodeConfig.DataHandlingConfig.DataAlignment = DMA_DATA_RIGHTALIGN_ZEROPADDED;

    // FIX: USART2는 Channel5 전용 리소스 사용
    if (HAL_DMAEx_List_BuildNode(&NodeConfig, &Node_GPDMA1_Channel5) != HAL_OK)
    {
      Error_Handler();
    }

    if (HAL_DMAEx_List_InsertNode(&List_GPDMA1_Channel5, NULL, &Node_GPDMA1_Channel5) != HAL_OK)
    {
      Error_Handler();
    }

    if (HAL_DMAEx_List_SetCircularMode(&List_GPDMA1_Channel5) != HAL_OK)
    {
      Error_Handler();
    }

    handle_GPDMA1_Channel5.Instance                         = GPDMA1_Channel5;
    handle_GPDMA1_Channel5.InitLinkedList.Priority          = DMA_LOW_PRIORITY_LOW_WEIGHT;
    handle_GPDMA1_Channel5.InitLinkedList.LinkStepMode      = DMA_LSM_FULL_EXECUTION;
    handle_GPDMA1_Channel5.InitLinkedList.LinkAllocatedPort = DMA_LINK_ALLOCATED_PORT0;
    handle_GPDMA1_Channel5.InitLinkedList.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
    handle_GPDMA1_Channel5.InitLinkedList.LinkedListMode    = DMA_LINKEDLIST_CIRCULAR;
    if (HAL_DMAEx_List_Init(&handle_GPDMA1_Channel5) != HAL_OK)
    {
      Error_Handler();
    }

    if (HAL_DMAEx_List_LinkQ(&handle_GPDMA1_Channel5, &List_GPDMA1_Channel5) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle, hdmarx, handle_GPDMA1_Channel5);

    if (HAL_DMA_ConfigChannelAttributes(&handle_GPDMA1_Channel5, DMA_CHANNEL_NPRIV) != HAL_OK)
    {
      Error_Handler();
    }
  }

  if (uartHandle->Instance == USART3)
  {
    /** Initializes the peripherals clock
     */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART3;
    PeriphClkInitStruct.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* USART3 clock enable */
    __HAL_RCC_USART3_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**USART3 GPIO Configuration
    PB10     ------> USART3_TX
    PD9      ------> USART3_RX
    */
    GPIO_InitStruct.Pin       = GPIO_PIN_10;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin       = GPIO_PIN_9;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* USART3 DMA Init */
    /* GPDMA2_REQUEST_USART3_RX Init */
    NodeConfig.NodeType                         = DMA_GPDMA_LINEAR_NODE;
    NodeConfig.Init.Request                     = GPDMA2_REQUEST_USART3_RX;
    NodeConfig.Init.BlkHWRequest                = DMA_BREQ_SINGLE_BURST;
    NodeConfig.Init.Direction                   = DMA_PERIPH_TO_MEMORY;
    NodeConfig.Init.SrcInc                      = DMA_SINC_FIXED;
    NodeConfig.Init.DestInc                     = DMA_DINC_INCREMENTED;
    NodeConfig.Init.SrcDataWidth                = DMA_SRC_DATAWIDTH_BYTE;
    NodeConfig.Init.DestDataWidth               = DMA_DEST_DATAWIDTH_BYTE;
    NodeConfig.Init.SrcBurstLength              = 1;
    NodeConfig.Init.DestBurstLength             = 1;
    NodeConfig.Init.TransferAllocatedPort       = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT0;
    NodeConfig.Init.TransferEventMode           = DMA_TCEM_BLOCK_TRANSFER;
    NodeConfig.Init.Mode                        = DMA_NORMAL;
    NodeConfig.TriggerConfig.TriggerPolarity    = DMA_TRIG_POLARITY_MASKED;
    NodeConfig.DataHandlingConfig.DataExchange  = DMA_EXCHANGE_NONE;
    NodeConfig.DataHandlingConfig.DataAlignment = DMA_DATA_RIGHTALIGN_ZEROPADDED;

    // FIX: USART3는 GPDMA2 Channel0 전용 리소스 사용
    if (HAL_DMAEx_List_BuildNode(&NodeConfig, &Node_GPDMA2_Channel0) != HAL_OK)
    {
      Error_Handler();
    }

    if (HAL_DMAEx_List_InsertNode(&List_GPDMA2_Channel0, NULL, &Node_GPDMA2_Channel0) != HAL_OK)
    {
      Error_Handler();
    }

    if (HAL_DMAEx_List_SetCircularMode(&List_GPDMA2_Channel0) != HAL_OK)
    {
      Error_Handler();
    }

    handle_GPDMA2_Channel0.Instance                         = GPDMA2_Channel0;
    handle_GPDMA2_Channel0.InitLinkedList.Priority          = DMA_LOW_PRIORITY_LOW_WEIGHT;
    handle_GPDMA2_Channel0.InitLinkedList.LinkStepMode      = DMA_LSM_FULL_EXECUTION;
    handle_GPDMA2_Channel0.InitLinkedList.LinkAllocatedPort = DMA_LINK_ALLOCATED_PORT0;
    handle_GPDMA2_Channel0.InitLinkedList.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
    handle_GPDMA2_Channel0.InitLinkedList.LinkedListMode    = DMA_LINKEDLIST_CIRCULAR;
    if (HAL_DMAEx_List_Init(&handle_GPDMA2_Channel0) != HAL_OK)
    {
      Error_Handler();
    }

    if (HAL_DMAEx_List_LinkQ(&handle_GPDMA2_Channel0, &List_GPDMA2_Channel0) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle, hdmarx, handle_GPDMA2_Channel0);

    if (HAL_DMA_ConfigChannelAttributes(&handle_GPDMA2_Channel0, DMA_CHANNEL_NPRIV) != HAL_OK)
    {
      Error_Handler();
    }
  }

  if (uartHandle->Instance == UART4)
  {
    /** Initializes the peripherals clock
     */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_UART4;
    PeriphClkInitStruct.Uart4ClockSelection  = RCC_UART4CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* UART4 clock enable */
    __HAL_RCC_UART4_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**UART4 GPIO Configuration
    PA0     ------> UART4_TX
    PD11     ------> UART4_RX
    */
    GPIO_InitStruct.Pin       = GPIO_PIN_0;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF8_UART4;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin       = GPIO_PIN_11;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF8_UART4;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* UART4 DMA Init */
    /* GPDMA1_REQUEST_UART4_RX Init */
    NodeConfig.NodeType                         = DMA_GPDMA_LINEAR_NODE;
    NodeConfig.Init.Request                     = GPDMA1_REQUEST_UART4_RX;
    NodeConfig.Init.BlkHWRequest                = DMA_BREQ_SINGLE_BURST;
    NodeConfig.Init.Direction                   = DMA_PERIPH_TO_MEMORY;
    NodeConfig.Init.SrcInc                      = DMA_SINC_FIXED;
    NodeConfig.Init.DestInc                     = DMA_DINC_INCREMENTED;
    NodeConfig.Init.SrcDataWidth                = DMA_SRC_DATAWIDTH_BYTE;
    NodeConfig.Init.DestDataWidth               = DMA_DEST_DATAWIDTH_BYTE;
    NodeConfig.Init.SrcBurstLength              = 1;
    NodeConfig.Init.DestBurstLength             = 1;
    NodeConfig.Init.TransferAllocatedPort       = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT0;
    NodeConfig.Init.TransferEventMode           = DMA_TCEM_BLOCK_TRANSFER;
    NodeConfig.Init.Mode                        = DMA_NORMAL;
    NodeConfig.TriggerConfig.TriggerPolarity    = DMA_TRIG_POLARITY_MASKED;
    NodeConfig.DataHandlingConfig.DataExchange  = DMA_EXCHANGE_NONE;
    NodeConfig.DataHandlingConfig.DataAlignment = DMA_DATA_RIGHTALIGN_ZEROPADDED;
    if (HAL_DMAEx_List_BuildNode(&NodeConfig, &Node_GPDMA1_Channel4) != HAL_OK)
    {
      Error_Handler();
    }

    if (HAL_DMAEx_List_InsertNode(&List_GPDMA1_Channel4, NULL, &Node_GPDMA1_Channel4) != HAL_OK)
    {
      Error_Handler();
    }

    if (HAL_DMAEx_List_SetCircularMode(&List_GPDMA1_Channel4) != HAL_OK)
    {
      Error_Handler();
    }

    handle_GPDMA1_Channel4.Instance                         = GPDMA1_Channel4;
    handle_GPDMA1_Channel4.InitLinkedList.Priority          = DMA_LOW_PRIORITY_LOW_WEIGHT;
    handle_GPDMA1_Channel4.InitLinkedList.LinkStepMode      = DMA_LSM_FULL_EXECUTION;
    handle_GPDMA1_Channel4.InitLinkedList.LinkAllocatedPort = DMA_LINK_ALLOCATED_PORT0;
    handle_GPDMA1_Channel4.InitLinkedList.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
    handle_GPDMA1_Channel4.InitLinkedList.LinkedListMode    = DMA_LINKEDLIST_CIRCULAR;
    if (HAL_DMAEx_List_Init(&handle_GPDMA1_Channel4) != HAL_OK)
    {
      Error_Handler();
    }

    if (HAL_DMAEx_List_LinkQ(&handle_GPDMA1_Channel4, &List_GPDMA1_Channel4) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle, hdmarx, handle_GPDMA1_Channel4);

    if (HAL_DMA_ConfigChannelAttributes(&handle_GPDMA1_Channel4, DMA_CHANNEL_NPRIV) != HAL_OK)
    {
      Error_Handler();
    }
  }

  if (uartHandle->Instance == UART9)
  {
    /** Initializes the peripherals clock
     */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_UART9;
    PeriphClkInitStruct.Uart9ClockSelection  = RCC_UART9CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* UART9 clock enable */
    __HAL_RCC_UART9_CLK_ENABLE();

    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**UART9 GPIO Configuration
    PD14     ------> UART9_RX
    PD15     ------> UART9_TX
    */
    GPIO_InitStruct.Pin       = GPIO_PIN_14 | GPIO_PIN_15;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF11_UART9;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* UART9 DMA Init */
    /* GPDMA1_REQUEST_UART9_RX Init */
    NodeConfig.NodeType                         = DMA_GPDMA_LINEAR_NODE;
    NodeConfig.Init.Request                     = GPDMA1_REQUEST_UART9_RX;
    NodeConfig.Init.BlkHWRequest                = DMA_BREQ_SINGLE_BURST;
    NodeConfig.Init.Direction                   = DMA_PERIPH_TO_MEMORY;
    NodeConfig.Init.SrcInc                      = DMA_SINC_FIXED;
    NodeConfig.Init.DestInc                     = DMA_DINC_INCREMENTED;
    NodeConfig.Init.SrcDataWidth                = DMA_SRC_DATAWIDTH_BYTE;
    NodeConfig.Init.DestDataWidth               = DMA_DEST_DATAWIDTH_BYTE;
    NodeConfig.Init.SrcBurstLength              = 1;
    NodeConfig.Init.DestBurstLength             = 1;
    NodeConfig.Init.TransferAllocatedPort       = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT0;
    NodeConfig.Init.TransferEventMode           = DMA_TCEM_BLOCK_TRANSFER;
    NodeConfig.Init.Mode                        = DMA_NORMAL;
    NodeConfig.TriggerConfig.TriggerPolarity    = DMA_TRIG_POLARITY_MASKED;
    NodeConfig.DataHandlingConfig.DataExchange  = DMA_EXCHANGE_NONE;
    NodeConfig.DataHandlingConfig.DataAlignment = DMA_DATA_RIGHTALIGN_ZEROPADDED;
    if (HAL_DMAEx_List_BuildNode(&NodeConfig, &Node_GPDMA1_Channel5) != HAL_OK)
    {
      Error_Handler();
    }

    if (HAL_DMAEx_List_InsertNode(&List_GPDMA1_Channel5, NULL, &Node_GPDMA1_Channel5) != HAL_OK)
    {
      Error_Handler();
    }

    if (HAL_DMAEx_List_SetCircularMode(&List_GPDMA1_Channel5) != HAL_OK)
    {
      Error_Handler();
    }

    handle_GPDMA1_Channel5.Instance                         = GPDMA1_Channel5;
    handle_GPDMA1_Channel5.InitLinkedList.Priority          = DMA_LOW_PRIORITY_LOW_WEIGHT;
    handle_GPDMA1_Channel5.InitLinkedList.LinkStepMode      = DMA_LSM_FULL_EXECUTION;
    handle_GPDMA1_Channel5.InitLinkedList.LinkAllocatedPort = DMA_LINK_ALLOCATED_PORT0;
    handle_GPDMA1_Channel5.InitLinkedList.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
    handle_GPDMA1_Channel5.InitLinkedList.LinkedListMode    = DMA_LINKEDLIST_CIRCULAR;
    if (HAL_DMAEx_List_Init(&handle_GPDMA1_Channel5) != HAL_OK)
    {
      Error_Handler();
    }

    if (HAL_DMAEx_List_LinkQ(&handle_GPDMA1_Channel5, &List_GPDMA1_Channel5) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle, hdmarx, handle_GPDMA1_Channel5);

    if (HAL_DMA_ConfigChannelAttributes(&handle_GPDMA1_Channel5, DMA_CHANNEL_NPRIV) != HAL_OK)
    {
      Error_Handler();
    }
  }

  /* ==== USART6 Init - UART_1, TX=PC6, RX=PC7 ==== */
  if (uartHandle->Instance == USART6)
  {
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART6;
    PeriphClkInitStruct.Usart6ClockSelection = RCC_USART6CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_RCC_USART6_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* USART6 GPIO Configuration */
    /* PC6     ------> USART6_TX */
    /* PC7     ------> USART6_RX */
    GPIO_InitStruct.Pin       = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART6;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* USART6 DMA Init */
    /* GPDMA1_REQUEST_USART6_RX Init */
    NodeConfig.NodeType                         = DMA_GPDMA_LINEAR_NODE;
    NodeConfig.Init.Request                     = GPDMA1_REQUEST_USART6_RX;
    NodeConfig.Init.BlkHWRequest                = DMA_BREQ_SINGLE_BURST;
    NodeConfig.Init.Direction                   = DMA_PERIPH_TO_MEMORY;
    NodeConfig.Init.SrcInc                      = DMA_SINC_FIXED;
    NodeConfig.Init.DestInc                     = DMA_DINC_INCREMENTED;
    NodeConfig.Init.SrcDataWidth                = DMA_SRC_DATAWIDTH_BYTE;
    NodeConfig.Init.DestDataWidth               = DMA_DEST_DATAWIDTH_BYTE;
    NodeConfig.Init.SrcBurstLength              = 1;
    NodeConfig.Init.DestBurstLength             = 1;
    NodeConfig.Init.TransferAllocatedPort       = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT0;
    NodeConfig.Init.TransferEventMode           = DMA_TCEM_BLOCK_TRANSFER;
    NodeConfig.Init.Mode                        = DMA_NORMAL;
    NodeConfig.TriggerConfig.TriggerPolarity    = DMA_TRIG_POLARITY_MASKED;
    NodeConfig.DataHandlingConfig.DataExchange  = DMA_EXCHANGE_NONE;
    NodeConfig.DataHandlingConfig.DataAlignment = DMA_DATA_RIGHTALIGN_ZEROPADDED;
    if (HAL_DMAEx_List_BuildNode(&NodeConfig, &Node_GPDMA1_Channel1) != HAL_OK)
    {
      Error_Handler();
    }

    if (HAL_DMAEx_List_InsertNode(&List_GPDMA1_Channel1, NULL, &Node_GPDMA1_Channel1) != HAL_OK)
    {
      Error_Handler();
    }

    if (HAL_DMAEx_List_SetCircularMode(&List_GPDMA1_Channel1) != HAL_OK)
    {
      Error_Handler();
    }

    handle_GPDMA1_Channel1.Instance                         = GPDMA1_Channel1;
    handle_GPDMA1_Channel1.InitLinkedList.Priority          = DMA_LOW_PRIORITY_LOW_WEIGHT;
    handle_GPDMA1_Channel1.InitLinkedList.LinkStepMode      = DMA_LSM_FULL_EXECUTION;
    handle_GPDMA1_Channel1.InitLinkedList.LinkAllocatedPort = DMA_LINK_ALLOCATED_PORT0;
    handle_GPDMA1_Channel1.InitLinkedList.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
    handle_GPDMA1_Channel1.InitLinkedList.LinkedListMode    = DMA_LINKEDLIST_CIRCULAR;
    if (HAL_DMAEx_List_Init(&handle_GPDMA1_Channel1) != HAL_OK)
    {
      Error_Handler();
    }

    if (HAL_DMAEx_List_LinkQ(&handle_GPDMA1_Channel1, &List_GPDMA1_Channel1) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle, hdmarx, handle_GPDMA1_Channel1);

    if (HAL_DMA_ConfigChannelAttributes(&handle_GPDMA1_Channel1, DMA_CHANNEL_NPRIV) != HAL_OK)
    {
      Error_Handler();
    }
  }

  /* ==== USART10 Init - UART_2, TX=PE3, RX=PE2 ==== */
  if (uartHandle->Instance == USART10)
  {
    PeriphClkInitStruct.PeriphClockSelection  = RCC_PERIPHCLK_USART10;
    PeriphClkInitStruct.Usart10ClockSelection = RCC_USART10CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_RCC_USART10_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();

    /* USART10 GPIO Configuration */
    /* PE3     ------> USART10_TX */
    /* PE2     ------> USART10_RX */
    GPIO_InitStruct.Pin       = GPIO_PIN_3 | GPIO_PIN_2;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART10;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    /* USART10 DMA Init */
    /* GPDMA1_REQUEST_USART10_RX Init */
    NodeConfig.NodeType                         = DMA_GPDMA_LINEAR_NODE;
    NodeConfig.Init.Request                     = GPDMA1_REQUEST_USART10_RX;
    NodeConfig.Init.BlkHWRequest                = DMA_BREQ_SINGLE_BURST;
    NodeConfig.Init.Direction                   = DMA_PERIPH_TO_MEMORY;
    NodeConfig.Init.SrcInc                      = DMA_SINC_FIXED;
    NodeConfig.Init.DestInc                     = DMA_DINC_INCREMENTED;
    NodeConfig.Init.SrcDataWidth                = DMA_SRC_DATAWIDTH_BYTE;
    NodeConfig.Init.DestDataWidth               = DMA_DEST_DATAWIDTH_BYTE;
    NodeConfig.Init.SrcBurstLength              = 1;
    NodeConfig.Init.DestBurstLength             = 1;
    NodeConfig.Init.TransferAllocatedPort       = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT0;
    NodeConfig.Init.TransferEventMode           = DMA_TCEM_BLOCK_TRANSFER;
    NodeConfig.Init.Mode                        = DMA_NORMAL;
    NodeConfig.TriggerConfig.TriggerPolarity    = DMA_TRIG_POLARITY_MASKED;
    NodeConfig.DataHandlingConfig.DataExchange  = DMA_EXCHANGE_NONE;
    NodeConfig.DataHandlingConfig.DataAlignment = DMA_DATA_RIGHTALIGN_ZEROPADDED;
    if (HAL_DMAEx_List_BuildNode(&NodeConfig, &Node_GPDMA1_Channel2) != HAL_OK)
    {
      Error_Handler();
    }

    if (HAL_DMAEx_List_InsertNode(&List_GPDMA1_Channel2, NULL, &Node_GPDMA1_Channel2) != HAL_OK)
    {
      Error_Handler();
    }

    if (HAL_DMAEx_List_SetCircularMode(&List_GPDMA1_Channel2) != HAL_OK)
    {
      Error_Handler();
    }

    handle_GPDMA1_Channel2.Instance                         = GPDMA1_Channel2;
    handle_GPDMA1_Channel2.InitLinkedList.Priority          = DMA_LOW_PRIORITY_LOW_WEIGHT;
    handle_GPDMA1_Channel2.InitLinkedList.LinkStepMode      = DMA_LSM_FULL_EXECUTION;
    handle_GPDMA1_Channel2.InitLinkedList.LinkAllocatedPort = DMA_LINK_ALLOCATED_PORT0;
    handle_GPDMA1_Channel2.InitLinkedList.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
    handle_GPDMA1_Channel2.InitLinkedList.LinkedListMode    = DMA_LINKEDLIST_CIRCULAR;
    if (HAL_DMAEx_List_Init(&handle_GPDMA1_Channel2) != HAL_OK)
    {
      Error_Handler();
    }

    if (HAL_DMAEx_List_LinkQ(&handle_GPDMA1_Channel2, &List_GPDMA1_Channel2) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle, hdmarx, handle_GPDMA1_Channel2);

    if (HAL_DMA_ConfigChannelAttributes(&handle_GPDMA1_Channel2, DMA_CHANNEL_NPRIV) != HAL_OK)
    {
      Error_Handler();
    }
  }

  /* ==== UART5 Init - RS485_1, TX=PB13, RX=PD2 ==== */
  if (uartHandle->Instance == UART5)
  {
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_UART5;
    PeriphClkInitStruct.Uart5ClockSelection  = RCC_UART5CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_RCC_UART5_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();

    /* UART5 GPIO Configuration */
    /* PB13    ------> UART5_TX */
    /* PD2     ------> UART5_RX */
    GPIO_InitStruct.Pin       = GPIO_PIN_13;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF8_UART5;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin       = GPIO_PIN_2;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF8_UART5;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* UART5 DMA Init */
    /* GPDMA1_REQUEST_UART5_RX Init */
    NodeConfig.NodeType                         = DMA_GPDMA_LINEAR_NODE;
    NodeConfig.Init.Request                     = GPDMA1_REQUEST_UART5_RX;
    NodeConfig.Init.BlkHWRequest                = DMA_BREQ_SINGLE_BURST;
    NodeConfig.Init.Direction                   = DMA_PERIPH_TO_MEMORY;
    NodeConfig.Init.SrcInc                      = DMA_SINC_FIXED;
    NodeConfig.Init.DestInc                     = DMA_DINC_INCREMENTED;
    NodeConfig.Init.SrcDataWidth                = DMA_SRC_DATAWIDTH_BYTE;
    NodeConfig.Init.DestDataWidth               = DMA_DEST_DATAWIDTH_BYTE;
    NodeConfig.Init.SrcBurstLength              = 1;
    NodeConfig.Init.DestBurstLength             = 1;
    NodeConfig.Init.TransferAllocatedPort       = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT0;
    NodeConfig.Init.TransferEventMode           = DMA_TCEM_BLOCK_TRANSFER;
    NodeConfig.Init.Mode                        = DMA_NORMAL;
    NodeConfig.TriggerConfig.TriggerPolarity    = DMA_TRIG_POLARITY_MASKED;
    NodeConfig.DataHandlingConfig.DataExchange  = DMA_EXCHANGE_NONE;
    NodeConfig.DataHandlingConfig.DataAlignment = DMA_DATA_RIGHTALIGN_ZEROPADDED;
    if (HAL_DMAEx_List_BuildNode(&NodeConfig, &Node_GPDMA1_Channel3) != HAL_OK)
    {
      Error_Handler();
    }

    if (HAL_DMAEx_List_InsertNode(&List_GPDMA1_Channel3, NULL, &Node_GPDMA1_Channel3) != HAL_OK)
    {
      Error_Handler();
    }

    if (HAL_DMAEx_List_SetCircularMode(&List_GPDMA1_Channel3) != HAL_OK)
    {
      Error_Handler();
    }

    handle_GPDMA1_Channel3.Instance                         = GPDMA1_Channel3;
    handle_GPDMA1_Channel3.InitLinkedList.Priority          = DMA_LOW_PRIORITY_LOW_WEIGHT;
    handle_GPDMA1_Channel3.InitLinkedList.LinkStepMode      = DMA_LSM_FULL_EXECUTION;
    handle_GPDMA1_Channel3.InitLinkedList.LinkAllocatedPort = DMA_LINK_ALLOCATED_PORT0;
    handle_GPDMA1_Channel3.InitLinkedList.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
    handle_GPDMA1_Channel3.InitLinkedList.LinkedListMode    = DMA_LINKEDLIST_CIRCULAR;
    if (HAL_DMAEx_List_Init(&handle_GPDMA1_Channel3) != HAL_OK)
    {
      Error_Handler();
    }

    if (HAL_DMAEx_List_LinkQ(&handle_GPDMA1_Channel3, &List_GPDMA1_Channel3) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle, hdmarx, handle_GPDMA1_Channel3);

    if (HAL_DMA_ConfigChannelAttributes(&handle_GPDMA1_Channel3, DMA_CHANNEL_NPRIV) != HAL_OK)
    {
      Error_Handler();
    }
  }

  /* ==== UART7 Init - RS232, TX=PE8, RX=PE7, RTS=PE9, CTS=PE10 ==== */
  if (uartHandle->Instance == UART7)
  {
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_UART7;
    PeriphClkInitStruct.Uart7ClockSelection  = RCC_UART7CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_RCC_UART7_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPDMA2_CLK_ENABLE();

    /* UART7 GPIO Configuration */
    /* PE8     ------> UART7_TX */
    /* PE7     ------> UART7_RX */
    /* PE9     ------> UART7_RTS */
    /* PE10    ------> UART7_CTS */
    GPIO_InitStruct.Pin       = GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_UART7;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    /* UART7 DMA Init */
    /* GPDMA2_REQUEST_UART7_RX Init */
    NodeConfig.NodeType                         = DMA_GPDMA_LINEAR_NODE;
    NodeConfig.Init.Request                     = GPDMA2_REQUEST_UART7_RX;
    NodeConfig.Init.BlkHWRequest                = DMA_BREQ_SINGLE_BURST;
    NodeConfig.Init.Direction                   = DMA_PERIPH_TO_MEMORY;
    NodeConfig.Init.SrcInc                      = DMA_SINC_FIXED;
    NodeConfig.Init.DestInc                     = DMA_DINC_INCREMENTED;
    NodeConfig.Init.SrcDataWidth                = DMA_SRC_DATAWIDTH_BYTE;
    NodeConfig.Init.DestDataWidth               = DMA_DEST_DATAWIDTH_BYTE;
    NodeConfig.Init.SrcBurstLength              = 1;
    NodeConfig.Init.DestBurstLength             = 1;
    NodeConfig.Init.TransferAllocatedPort       = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT0;
    NodeConfig.Init.TransferEventMode           = DMA_TCEM_BLOCK_TRANSFER;
    NodeConfig.Init.Mode                        = DMA_NORMAL;
    NodeConfig.TriggerConfig.TriggerPolarity    = DMA_TRIG_POLARITY_MASKED;
    NodeConfig.DataHandlingConfig.DataExchange  = DMA_EXCHANGE_NONE;
    NodeConfig.DataHandlingConfig.DataAlignment = DMA_DATA_RIGHTALIGN_ZEROPADDED;
    if (HAL_DMAEx_List_BuildNode(&NodeConfig, &Node_GPDMA2_Channel1) != HAL_OK)
    {
      Error_Handler();
    }

    if (HAL_DMAEx_List_InsertNode(&List_GPDMA2_Channel1, NULL, &Node_GPDMA2_Channel1) != HAL_OK)
    {
      Error_Handler();
    }

    if (HAL_DMAEx_List_SetCircularMode(&List_GPDMA2_Channel1) != HAL_OK)
    {
      Error_Handler();
    }

    handle_GPDMA2_Channel1.Instance                         = GPDMA2_Channel1;
    handle_GPDMA2_Channel1.InitLinkedList.Priority          = DMA_LOW_PRIORITY_LOW_WEIGHT;
    handle_GPDMA2_Channel1.InitLinkedList.LinkStepMode      = DMA_LSM_FULL_EXECUTION;
    handle_GPDMA2_Channel1.InitLinkedList.LinkAllocatedPort = DMA_LINK_ALLOCATED_PORT0;
    handle_GPDMA2_Channel1.InitLinkedList.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
    handle_GPDMA2_Channel1.InitLinkedList.LinkedListMode    = DMA_LINKEDLIST_CIRCULAR;
    if (HAL_DMAEx_List_Init(&handle_GPDMA2_Channel1) != HAL_OK)
    {
      Error_Handler();
    }

    if (HAL_DMAEx_List_LinkQ(&handle_GPDMA2_Channel1, &List_GPDMA2_Channel1) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle, hdmarx, handle_GPDMA2_Channel1);

    if (HAL_DMA_ConfigChannelAttributes(&handle_GPDMA2_Channel1, DMA_CHANNEL_NPRIV) != HAL_OK)
    {
      Error_Handler();
    }
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *uartHandle)
{
  if (uartHandle->Instance == USART1)
  {
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();

    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PB7     ------> USART1_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9);
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_7);

    /* USART1 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmarx);
  }

  if (uartHandle->Instance == USART2)
  {
    /* Peripheral clock disable */
    __HAL_RCC_USART2_CLK_DISABLE();

    /**USART2 GPIO Configuration
    PD5     ------> USART2_TX
    PD6     ------> USART2_RX
    */
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_5 | GPIO_PIN_6);

    /* USART2 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmarx);
  }

  if (uartHandle->Instance == USART3)
  {
    /* Peripheral clock disable */
    __HAL_RCC_USART3_CLK_DISABLE();

    /**USART3 GPIO Configuration
    PB10     ------> USART3_TX
    PD9     ------> USART3_RX
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_10);
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_9);

    /* USART3 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmarx);
  }

  if (uartHandle->Instance == UART4)
  {
    /* Peripheral clock disable */
    __HAL_RCC_UART4_CLK_DISABLE();

    /**UART4 GPIO Configuration
    PA0     ------> UART4_TX
    PD11     ------> UART4_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_0);
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_11);

    /* UART4 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmarx);
  }

  if (uartHandle->Instance == UART9)
  {
    /* Peripheral clock disable */
    __HAL_RCC_UART9_CLK_DISABLE();

    /**UART9 GPIO Configuration
    PD14     ------> UART9_RX
    PD15     ------> UART9_TX
    */
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_14 | GPIO_PIN_15);

    /* UART9 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmarx);
  }

  // UART_1
  if (uartHandle->Instance == USART6)
  {
    __HAL_RCC_USART6_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_6 | GPIO_PIN_7);
    HAL_DMA_DeInit(uartHandle->hdmarx);
  }

  // UART_2
  if (uartHandle->Instance == USART10)
  {
    __HAL_RCC_USART10_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOE, GPIO_PIN_2 | GPIO_PIN_3);
    HAL_DMA_DeInit(uartHandle->hdmarx);
  }

  /* ==== UART5 DeInit - RS485_1 ==== */
  if (uartHandle->Instance == UART5)
  {
    __HAL_RCC_UART5_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_13);
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_2);
    HAL_DMA_DeInit(uartHandle->hdmarx);
  }

  /* ==== UART7 DeInit - RS232 ==== */
  if (uartHandle->Instance == UART7)
  {
    __HAL_RCC_UART7_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOE, GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10);
    HAL_DMA_DeInit(uartHandle->hdmarx);
  }
}

#if CLI_USE(HW_UART)
void cliUart(cli_args_t *args)
{
  bool ret = false;


  if (args->argc == 1 && args->isStr(0, "info"))
  {
    for (int i = 0; i < UART_MAX_CH; i++)
    {
      cliPrintf("_DEF_UART%d : %s, %d bps, init[%d]\n", i + 1, uart_hw_tbl[i].p_msg, uartGetBaud(i), uart_tbl[i].is_open);
    }
    ret = true;
  }

  if (args->argc == 3 && args->isStr(0, "open"))
  {
    uint8_t  uart_ch;
    uint32_t uart_baud;

    uart_ch   = constrain(args->getData(1), 1, UART_MAX_CH) - 1;
    uart_baud = args->getData(2);

    if (uart_ch != cliGetPort())
    {
      uartOpen(uart_ch, uart_baud);

      cliPrintf("_DEF_UART%d : %s, %d bps\n", uart_ch + 1, uart_hw_tbl[uart_ch].p_msg, uartGetBaud(uart_ch));
    }
    ret = true;
  }

  if (args->argc == 2 && args->isStr(0, "test"))
  {
    uint8_t uart_ch;

    uart_ch = constrain(args->getData(1), 1, UART_MAX_CH) - 1;

    if (uart_ch != cliGetPort())
    {
      uint8_t rx_data;

      // LIN 타입 테스트 시 9600으로 열기
      if (uart_hw_tbl[uart_ch].uart_type == UART_TYPE_LIN)
      {
        uartOpen(uart_ch, 9600);
        cliPrintf("LIN Open ch%d, %d bps\n", uart_ch + 1, uartGetBaud(uart_ch));
      }

      while (1)
      {
        if (uartAvailable(uart_ch) > 0)
        {
          rx_data = uartRead(uart_ch);
          cliPrintf("<- _DEF_UART%d RX : 0x%X\n", uart_ch + 1, rx_data);
        }

        if (cliAvailable() > 0)
        {
          rx_data = cliRead();
          if (rx_data == 'q')
          {
            break;
          }
          else
          {
            uartWrite(uart_ch, &rx_data, 1);
            cliPrintf("-> _DEF_UART%d TX : 0x%X\n", uart_ch + 1, rx_data);
          }
        }
      }

      // LIN 종료 시 복원
      if (uart_hw_tbl[uart_ch].uart_type == UART_TYPE_LIN)
      {
        uartClose(uart_ch);
        // LIN_3은 UART4를 RS485_2와 공유하고 있으므로, 복구 작업을 해주어야 한다.
        if (uart_ch == HW_UART_CH_LIN_3)
        {
          uartOpen(HW_UART_CH_RS485_2, 115200);
          cliPrintf("RS485_2 restored\n");
        }
      }
    }
    else
    {
      cliPrintf("This is cliPort\n");
    }
    ret = true;
  }

  if (ret == false)
  {
    cliPrintf("uart info\n");
    cliPrintf("uart open ch[1~%d] baud\n", HW_UART_MAX_CH);
    cliPrintf("uart test ch[1~%d]\n", HW_UART_MAX_CH);
  }
}
#endif


#endif
