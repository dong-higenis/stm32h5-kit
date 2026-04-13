#ifndef PERI_DEF_H_
#define PERI_DEF_H_

#define PERI_QUEUE_DEPTH     32
#define PERI_CAN_QUEUE_DEPTH 128

#include "ap_def.h"

/**
 * @brief 현재 들어온 데이터의 통신 프로토콜
 */
typedef enum
{
  PERI_PROTO_CAN = 0,
  PERI_PROTO_RS485,
  PERI_PROTO_RS232,
  PERI_PROTO_LIN,
  PERI_PROTO_UART,
  PERI_PROTO_MAX
} PeriProto_t;

/**
 * @brief 통신 프로토콜 별 채널에 따라 이름 별도 정의
 */
typedef enum
{
  PERI_CAN_0 = 0,
  PERI_CAN_1,
  PERI_RS485_0,
  PERI_RS485_1,
  PERI_RS232_0,
  PERI_LIN_0,
  PERI_LIN_1,
  PERI_LIN_2,
  PERI_UART_0,
  PERI_UART_1,
  PERI_MAX
} PeriName_t;

typedef enum
{
  PERI_DIR_RX = 0,
  PERI_DIR_TX
} PeriDir_t;

/**
 * @brief 프토토콜별 메시지 구조체 
 */
typedef struct
{
  PeriName_t name;
  PeriDir_t  dir;
  uint32_t   timestamp;

  can_msg_t  message;
  uint32_t   err_code;
} peri_can_msg_t;

typedef struct
{
  PeriName_t name;
  PeriDir_t  dir;
  uint32_t   timestamp;

  uint8_t    data[256];
  uint16_t   length;
} peri_serial_msg_t;

typedef struct
{
  PeriName_t name;
  PeriDir_t  dir;
  uint32_t   timestamp;

  uint8_t    id;
  uint8_t    data[8];
  uint8_t    length;
  uint8_t    err_code;
} peri_lin_msg_t;

/**
 * @brief 각 통신별 채널 정보 테이블
 */
typedef struct
{
  PeriName_t  name;    // 통신 이름
  PeriProto_t proto;   // 통신 프로토콜
  uint8_t     hw_ch;   // 통신 하드웨어 채널
  const char *str;     // 통신 이름 문자열
  uint16_t    q_depth; // 큐 depth
  uint16_t    q_size;  // 큐 타겟 데이터 size
} peri_ch_info_t;

extern const peri_ch_info_t peri_ch_tbl[PERI_MAX];

#endif