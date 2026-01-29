#include "custom_can.h"
#include "fdcan.h"

#define CAN_80M_EXAMPLE_PRESCALER       10 // 분주비: 80MHz를 몇으로 나눌지 (10 = 8MHz로 분주)
#define CAN_80M_EXAMPLE_SYNC_JUMP_WIDTH 8  // 동기 점프 폭: 클럭 오차 보정 가능 범위 (최대 8 Time Quantum)
#define CAN_80M_EXAMPLE_T_SEG_1         13 // 타임 세그먼트 1: 샘플 포인트 이전 구간 길이 즉 신호를 읽기 전 대기 시간(13 Time Quantum)
#define CAN_80M_EXAMPLE_T_SEG_2         2  // 타임 세그먼트 2: 샘플 포인트 이후 구간 길이 즉 신호를 읽은 후 다음 비트까지 대기 시간(2 Time Quantum)

                                           /**
                                            * @details 참조 : 계산공식
                                            *
                                            *  - 1 비트 시간 = (1 + TSEG1 + TSEG2) = (1 + 13 + 2) = 16
                                            *
                                            *  - Baudrate   = 80MHz / (Prescaler × 1 비트 시간)
                                            *               = 80,000,000 / (10 × 16)
                                            *               = 500,000 bps (500 Kbps)
                                            *
                                            *  - 샘플 포인트 = (1 + TSEG1) / (1 + TSEG1 + TSEG2) × 100%
                                            *               = (1 + 13) / 16 × 100%
                                            *               = 87.5%
                                            */


bool customCanInit(FDCAN_HandleTypeDef *can_handler, bool is_loopback)
{
  if (can_handler == NULL)
  {
    return false;
  }

  FDCAN_HandleTypeDef *p_can = can_handler;

  customCanClose(p_can); // 기존에 can이 init되어있을수 있으니 닫아준다.

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
    return false;                                    // 잘못된 핸들러
  }

  p_can->Init.ClockDivider = FDCAN_CLOCK_DIV1;       // 클럭 분주 없음 (80MHz 그대로 사용)
  p_can->Init.FrameFormat  = FDCAN_FRAME_CLASSIC;    // Classical CAN 모드 (CAN 2.0)

  if (is_loopback)
  {
    p_can->Init.Mode = FDCAN_MODE_INTERNAL_LOOPBACK; // 내부 루프백 (자체 테스트용)
  }
  else
  {
    p_can->Init.Mode = FDCAN_MODE_NORMAL;            // 정상 통신 모드
  }

  p_can->Init.AutoRetransmission   = DISABLE;        // 자동 재전송 비활성화 (실패 시 재전송 안함)
  p_can->Init.TransmitPause        = ENABLE;         // 전송 간 일시정지 활성화 (버스 안정성)
  p_can->Init.ProtocolException    = ENABLE;         // 프로토콜 예외 처리 활성화
  p_can->Init.NominalPrescaler     = CAN_80M_EXAMPLE_PRESCALER;
  p_can->Init.NominalSyncJumpWidth = CAN_80M_EXAMPLE_SYNC_JUMP_WIDTH;
  p_can->Init.NominalTimeSeg1      = CAN_80M_EXAMPLE_T_SEG_1;
  p_can->Init.NominalTimeSeg2      = CAN_80M_EXAMPLE_T_SEG_2;

  p_can->Init.StdFiltersNbr   = 28;                      // 표준 ID 필터 개수 (11비트)
  p_can->Init.ExtFiltersNbr   = 8;                       // 확장 ID 필터 개수 (29비트)
  p_can->Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION; // TX는 FIFO 모드 사용 : 먼저 넣은 메시지가 먼저 나감

  if (HAL_FDCAN_Init(p_can) != HAL_OK)                   // 위에서 설정한 값들을 하드웨어에 적용
  {
    return false;
  }

  FDCAN_FilterTypeDef f = {0};

  f.IdType       = FDCAN_STANDARD_ID;
  f.FilterIndex  = 0;
  f.FilterType   = FDCAN_FILTER_MASK;
  f.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
  f.FilterID1    = 0x000; // ID
  f.FilterID2    = 0x000; // MASK (0이면 전부 match)

  if (HAL_FDCAN_ConfigFilter(p_can, &f) != HAL_OK) // 필터 설정
  {
	return false; 
  }

  if (HAL_FDCAN_Start(p_can) != HAL_OK) // hal 시작
  {
    return false;
  }

  return true; // 성공: 이제 CAN 통신 가능
}

void customCanClose(FDCAN_HandleTypeDef *can_handler)
{
  if (can_handler == NULL)
  {
    return;
  }

  HAL_FDCAN_Stop(can_handler);   // can 중지
  HAL_FDCAN_DeInit(can_handler); // can 초기화 해제
}
