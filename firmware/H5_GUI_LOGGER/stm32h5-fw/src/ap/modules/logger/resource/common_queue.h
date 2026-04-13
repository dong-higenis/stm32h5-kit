#ifndef COMMON_QUEUE_H_
#define COMMON_QUEUE_H_

#include "ap_def.h"

bool          commonQueueCreate(PeriName_t name, uint16_t depth, uint16_t item_size);
QueueHandle_t commonQueueGet(PeriName_t name);

#endif