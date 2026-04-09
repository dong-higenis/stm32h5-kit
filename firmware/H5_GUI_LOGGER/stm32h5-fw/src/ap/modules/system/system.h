#ifndef SYSTEM_H_
#define SYSTEM_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "ap_def.h"



bool systemInit(void);
void systemMain(void);
bool systemWaitStart(void);

#ifdef __cplusplus
}
#endif

#endif