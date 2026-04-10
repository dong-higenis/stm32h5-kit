#ifndef PERI_DEF_H_
#define PERI_DEF_H_

#define PERI_CAN_QUEUE_DEPTH 32

#include "ap_def.h"

typedef enum
{
  PERI_DIR_RX = 0,
  PERI_DIR_TX
} Peri_dir_t;

typedef struct 
{
  uint8_t      ch;
  Peri_dir_t   dir;
  uint32_t     timestamp;
  uint32_t     err_code; // 수신 시점 에러 상태
  can_msg_t    message;
} peri_can_msg_t;

#endif