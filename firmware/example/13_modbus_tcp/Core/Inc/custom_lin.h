#ifndef CUSTOM_LIN_H
#define CUSTOM_LIN_H

#include "usart.h" // uart 조작을 위해
#include "main.h"  // define 참조를 위해
#include "cli.h"

void customLinInit(void);
void cliLin(cli_args_t *args);

#endif
