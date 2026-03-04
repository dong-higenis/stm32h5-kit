#ifndef CUSTOM_FRAM_H
#define CUSTOM_FRAM_H

#include <stdbool.h>
#include <i2c.h>
#include "cli.h"

bool customFramInit(void);
bool customFramWrite(uint16_t addr, uint8_t data);
bool customFramRead(uint16_t addr, uint8_t *data);
void cliFram(cli_args_t *args); // 추가

#endif
