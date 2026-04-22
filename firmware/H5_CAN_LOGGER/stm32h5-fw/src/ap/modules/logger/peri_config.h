#ifndef PERI_CONFIG_H_
#define PERI_CONFIG_H_

#define PERI_CAN_QUEUE_DEPTH 128

#include "ap_def.h"

typedef enum
{
  PERI_PROTO_CAN = 0,
  PERI_PROTO_MAX
} PeriProto_t;

typedef enum
{
  PERI_CAN_1 = 0,
  PERI_CAN_2,
  PERI_MAX
} PeriName_t;

typedef enum
{
  PERI_DIR_RX = 0,
  PERI_DIR_TX
} PeriDir_t;

/**
 * @brief can log에 필요한 데이터 형식
 */
typedef struct
{
  PeriName_t name;
  PeriDir_t  dir;
  uint32_t   timestamp;

  can_msg_t message;
  uint32_t  err_code;
} peri_can_msg_t;

/**
 * @brief peri 모듈에 필요한 최소한의 정보
 */
typedef struct
{
  PeriName_t  name;
  PeriProto_t proto;
  uint8_t     hw_ch;
  uint16_t    q_depth;
  uint16_t    q_size;
} peri_ch_info_t;

const peri_ch_info_t *periGetChInfo(PeriName_t name);
PeriProto_t           periGetProto(PeriName_t name);
uint8_t               periGetHwCh(PeriName_t name);

#endif
