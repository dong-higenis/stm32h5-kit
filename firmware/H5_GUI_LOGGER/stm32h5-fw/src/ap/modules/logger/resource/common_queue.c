#include "common_queue.h"

static QueueHandle_t queue[PERI_MAX] = {NULL};

/**
 * @brief 호출자가 요청하면 queue를 만든다.
 */
bool commonQueueCreate(PeriName_t peri_name, uint16_t depth, uint16_t item_size)
{
  if (peri_name >= PERI_MAX)
    return false;

  queue[peri_name] = xQueueCreate(depth, item_size);
  if (queue[peri_name] == NULL)
  {
    logPrintf("[ERR] QueueCreate: queue[%d] failed\n", peri_name);
    return false;
  }

  return true;
}

// getter
QueueHandle_t commonQueueGet(PeriName_t peri_name)
{
  if (peri_name >= PERI_MAX)
    return NULL;

  return queue[peri_name];
}