#ifndef WRITER_H_
#define WRITER_H_

#include "ap_def.h"
#include "peri_config.h"

#define WRITER_LINE_MAX              (32 + (PERI_SERIAL_MAX_BUFF_LEN * 3))
#define WRITER_MAX_DEQUEUE_PER_PASS  8
#define WRITER_FORCE_FLUSH_PERIOD_MS 500

void writerRequestPause(void);
void writerRequestResume(void);

#endif
