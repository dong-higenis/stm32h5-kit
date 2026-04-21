#ifndef PERI_CONFIG_H_
#define PERI_CONFIG_H_

#define PERI_QUEUE_DEPTH     32
#define PERI_CAN_QUEUE_DEPTH 128

#include "ap_def.h"

/**
 * @brief Serial
 */
#define PERI_UART_BAUD           115200
#define PERI_UART_PACKET_USE_LF  1 // \n을 패킷 구분자로 씀
#define PERI_UART_PACKET_USE_CR  1 // \r을 패킷 구분자로 씀

#if !PERI_UART_PACKET_USE_CR && !PERI_UART_PACKET_USE_LF
#error "PERI_UART_PACKET_USE_CR or PERI_UART_PACKET_USE_LF must be 1"
#endif

#define PERI_SERIAL_MAX_BUFF_LEN 256

/**
 * @brief 현재 들어오는 데이터의 통신 프로토콜
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
 * @brief 통신 프로토콜 및 채널별로 이름 별도 정의
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
 * @brief 프로토콜별 메시지 구조체
 */
typedef struct
{
  PeriName_t name;
  PeriDir_t  dir;
  uint32_t   timestamp;

  can_msg_t message;
  uint32_t  err_code;
} peri_can_msg_t;

typedef struct
{
  PeriName_t name;
  PeriDir_t  dir;
  uint32_t   timestamp;

  uint8_t  data[PERI_SERIAL_MAX_BUFF_LEN];
  uint16_t length;
} peri_serial_msg_t;

typedef struct
{
  PeriName_t name;
  PeriDir_t  dir;
  uint32_t   timestamp;

  uint8_t id;
  uint8_t data[8];
  uint8_t length;
  uint8_t err_code;
} peri_lin_msg_t;

/**
 * @brief 각 통신별 채널 정보 테이블
 */
typedef struct
{
  PeriName_t  name;
  PeriProto_t proto;
  uint8_t     hw_ch;
  uint16_t    q_depth;
  uint16_t    q_size;
} peri_ch_info_t;

/**
 * @brief getter
 */
const peri_ch_info_t *periGetChInfo(PeriName_t name);
PeriProto_t           periGetProto(PeriName_t name);
uint8_t               periGetHwCh(PeriName_t name);

#endif
