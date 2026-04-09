#include "system.h"



#define lock(x)      xSemaphoreTake(x, portMAX_DELAY);
#define unLock(x)    xSemaphoreGive(x);


static SemaphoreHandle_t mutex_start;


bool systemInit(void)
{
  mutex_start = xSemaphoreCreateMutex();
  assert(mutex_start);  
  
  return true;
}

bool systemWaitStart(void)
{
  assert(mutex_start);

  lock(mutex_start);
  unLock(mutex_start);
  return true;
}

void systemMain(void)
{
  bool init_ret = true;

  lock(mutex_start);

  init_ret &= moduleInit();
  init_ret &= threadBegin();

  logBoot(false);
  logPrintf("[%s] Thread Started : SYSTEM\n", init_ret ? "OK":"E_" );
  unLock(mutex_start);

  while(1)
  {
    ledToggle(_DEF_LED1);

    if (init_ret)
    {
      delay(500);
    }
    else
    {
      delay(50);
    }
  }
}