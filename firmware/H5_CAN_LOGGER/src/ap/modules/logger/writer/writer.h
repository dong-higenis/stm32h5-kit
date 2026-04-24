#ifndef WRITER_H_
#define WRITER_H_

#include "ap_def.h"

#define WRITER_LINE_MAX              2048
#define WRITER_FORCE_FLUSH_PERIOD_MS 500
#define WRITER_CAN_DATA_COLUMN       64

void writerRequestPause(void);
void writerRequestResume(void);

#endif
