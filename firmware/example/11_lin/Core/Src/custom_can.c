#include "custom_can.h"
#include "fdcan.h"

extern UART_HandleTypeDef huart1;

bool    can_block_mode_req  = false;

#define CAN_80M_EXAMPLE_PRESCALER       10 
#define CAN_80M_EXAMPLE_SYNC_JUMP_WIDTH 8  
#define CAN_80M_EXAMPLE_T_SEG_1         13 
#define CAN_80M_EXAMPLE_T_SEG_2         2  

bool customCanInit(FDCAN_HandleTypeDef *can_handler, bool is_loopback)
{
  if (can_handler == NULL)
  {
    return false;
  }

  FDCAN_HandleTypeDef *p_can = can_handler;

  customCanClose(p_can); 

  if (can_handler == &hfdcan1)
  {
    p_can->Instance = FDCAN1;
  }
  else if (can_handler == &hfdcan2)
  {
    p_can->Instance = FDCAN2;
  }
  else
  {
    return false;                                   
  }

  p_can->Init.ClockDivider = FDCAN_CLOCK_DIV1;       
  p_can->Init.FrameFormat  = FDCAN_FRAME_CLASSIC;    

  if (is_loopback)
  {
    p_can->Init.Mode = FDCAN_MODE_INTERNAL_LOOPBACK; 
  }
  else
  {
    p_can->Init.Mode = FDCAN_MODE_NORMAL;           
  }

  p_can->Init.AutoRetransmission   = DISABLE;        
  p_can->Init.TransmitPause        = ENABLE;         
  p_can->Init.ProtocolException    = ENABLE;        
  p_can->Init.NominalPrescaler     = CAN_80M_EXAMPLE_PRESCALER;
  p_can->Init.NominalSyncJumpWidth = CAN_80M_EXAMPLE_SYNC_JUMP_WIDTH;
  p_can->Init.NominalTimeSeg1      = CAN_80M_EXAMPLE_T_SEG_1;
  p_can->Init.NominalTimeSeg2      = CAN_80M_EXAMPLE_T_SEG_2;

  p_can->Init.StdFiltersNbr   = 28;                   
  p_can->Init.ExtFiltersNbr   = 8;             
  p_can->Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION; 

  if (HAL_FDCAN_Init(p_can) != HAL_OK)     
  {
    return false;
  }

  FDCAN_FilterTypeDef f = {0};

  f.IdType       = FDCAN_STANDARD_ID;
  f.FilterIndex  = 0;
  f.FilterType   = FDCAN_FILTER_MASK;
  f.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
  f.FilterID1    = 0x000; 
  f.FilterID2    = 0x000; 

  if (HAL_FDCAN_ConfigFilter(p_can, &f) != HAL_OK) 
  {
	return false; 
  }

  if (HAL_FDCAN_Start(p_can) != HAL_OK) 
  {
    return false;
  }

  return true; 
}

void customCanClose(FDCAN_HandleTypeDef *can_handler)
{
  if (can_handler == NULL)
  {
    return;
  }

  HAL_FDCAN_Stop(can_handler); 
  HAL_FDCAN_DeInit(can_handler); 
}

/**
 * @brief CAN 테스트 명령어
 *        사용법: can test 1 (CAN1 테스트)
 *               can test 2 (CAN2 테스트)
 */
void cliCan(cli_args_t *args)
{
  if (args->argc < 2)
  {
    cliPrintf("Usage: can test ch\r\n");
    cliPrintf("ch: [1 ~ 2]\r\n");
    return;
  }

  if (strcmp(args->argv[0], "test") != 0)
  {
    cliPrintf("Unknown command. Use 'can test ch'\r\n");
    return;
  }

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

  FDCAN_HandleTypeDef *p_can = (ch == 1) ? &hfdcan1 : &hfdcan2;

#if _USE_CAN_LOOPBACK_MODE
  if (!customCanInit(p_can, true))           // 루프백 모드
  {
    cliPrintf("Error: CAN init failed\r\n"); // can 초기화 실패
    return;
  }
  cliPrintf("CAN%d started in LOOPBACK mode\r\n", ch);
#else
  if (!customCanInit(p_can, false)) 
  {
    cliPrintf("Error: CAN init failed\r\n");
    return;
  }
  cliPrintf("CAN%d started in NORMAL mode\r\n", ch);
#endif

  can_block_mode_req = true;
  HAL_UART_AbortReceive_IT(&huart1); 

  uint8_t uart_rx_data;

  while (1)
  {
    HAL_StatusTypeDef ret = HAL_UART_Receive(&huart1, &uart_rx_data, 1, 10);

    if (ret == HAL_OK)
    {
      if (uart_rx_data == '\r' || uart_rx_data == '\n')
      {
        break;
      }

      FDCAN_TxHeaderTypeDef tx_header;
      uint8_t               tx_data[8] = {0};

      tx_data[0] = uart_rx_data;


      tx_header.Identifier          = 0x123;             
      tx_header.IdType              = FDCAN_STANDARD_ID; 
      tx_header.TxFrameType         = FDCAN_DATA_FRAME;  
      tx_header.DataLength          = FDCAN_DLC_BYTES_8;
      tx_header.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
      tx_header.BitRateSwitch       = FDCAN_BRS_OFF;     
      tx_header.FDFormat            = FDCAN_CLASSIC_CAN; 
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

  customCanClose(p_can);

  can_block_mode_req = false;
  HAL_UART_Receive_IT(&huart1, &debug_uart_rx_char, 1); 
}
