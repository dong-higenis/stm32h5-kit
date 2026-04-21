#ifndef FILE_CONTROLLER_H__
#define FILE_CONTROLLER_H__

#include "ap_def.h"

#include "peri_config.h"
// -- 
// SD 기록 설정 값
#define FILE_CTRL_BUF_SIZE        1024
#define FILE_CTRL_FLUSH_THRESHOLD 512

bool fileCtrlInit(void);

bool fileCtrlOpen(PeriName_t name, const char *file_path, const char *csv_header);
void fileCtrlClose(PeriName_t name);
void fileCtrlWrite(PeriName_t name, const char *p_data, uint16_t length);
void fileCtrlFlush(PeriName_t name);
void fileCtrlFlushAll(void);

bool fileCtrlIsOpen(PeriName_t name);

#endif
