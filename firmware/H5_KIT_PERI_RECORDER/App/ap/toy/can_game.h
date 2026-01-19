#ifndef CAN_GAME_H
#define CAN_GAME_H

#include "ap_def.h"

typedef struct
{
  uint32_t id;
  uint8_t  len;
  uint8_t  data[8];
  uint32_t last_time;
  uint32_t rx_count;
  bool     valid;
} can_monitor_t;

extern can_monitor_t    can_monitor;

void canToyInit(void);
void canToyUpdate(uint8_t ch);

#endif
