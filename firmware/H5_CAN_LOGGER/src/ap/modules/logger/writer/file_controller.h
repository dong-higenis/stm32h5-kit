#ifndef FILE_CONTROLLER_H__
#define FILE_CONTROLLER_H__

#include "ap_def.h"

#define FILE_CTRL_BUF_SIZE        4096
#define FILE_CTRL_FLUSH_THRESHOLD 2048

#define FILE_CTRL_MAX_FILE_SIZE   (5 * 1024 * 1024)
#define FILE_CTRL_MAX_PATH_LEN    64
#define FILE_CTRL_MAX_FILE_INDEX  100

bool fileCtrlInit(void);

bool fileCtrlOpen(PeriName_t name, const char *file_path, const char *csv_header);
void fileCtrlClose(PeriName_t name);
bool fileCtrlWrite(PeriName_t name, const char *p_data, uint16_t length);
void fileCtrlFlush(PeriName_t name);
void fileCtrlFlushAll(void);
void fileCtrlResetLogStart(void);

#endif
