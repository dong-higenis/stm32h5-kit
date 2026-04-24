#ifndef FILE_CONTROLLER_H__
#define FILE_CONTROLLER_H__

#include "ap_def.h"

#define FILE_CTRL_BUF_SIZE        8192
#define FILE_CTRL_FLUSH_THRESHOLD 4096



bool fileCtrlInit(PeriName_t name, const char *file_path, const char *csv_header);
bool fileCtrlOpen(PeriName_t name, const rtc_timestamp_t *p_ts);
bool fileCtrlWrite(PeriName_t name, const rtc_timestamp_t *p_ts, const char *p_data, uint16_t length);
void fileCtrlClose(PeriName_t name);
void fileCtrlFlush(PeriName_t name);
void fileCtrlFlushAll(void);
void fileCtrlResetLogStart(void);

#endif
