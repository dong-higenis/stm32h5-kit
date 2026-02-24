#ifndef CUSTOM_SD_H
#define CUSTOM_SD_H

#include <stdbool.h>

#include "cli.h" // cli 사용을 위해

bool customSdInit(void);
void cliSdCard(cli_args_t *args);

#endif
