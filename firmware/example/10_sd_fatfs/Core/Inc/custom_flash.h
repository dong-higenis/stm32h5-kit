#ifndef CUSTOM_FLASH_H
#define CUSTOM_FLASH_H

#include <stdbool.h> // boolean 사용을 위해
#include <stdint.h>
#include <string.h>
#include "cli.h"

bool customFlashInit(void);
bool customFlashRead(uint32_t addr, uint8_t *read_data, uint32_t length);
bool customFlashWrite(uint32_t addr, uint8_t *write_data, uint32_t length);
bool customFlashSectorErase(uint32_t addr);
uint8_t customFlashReadStatus(void);
void cliFlash(cli_args_t *args);

#endif
