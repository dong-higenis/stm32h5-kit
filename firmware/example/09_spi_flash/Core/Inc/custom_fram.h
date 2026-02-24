#ifndef CUSTOM_FRAM_H
#define CUSTOM_FRAM_H

#include <stdbool.h>
#include <i2c.h>

bool customFramInit(void);
bool customFramWrite(uint16_t addr, uint8_t data);
bool customFramRead(uint16_t addr, uint8_t *data);

#endif
