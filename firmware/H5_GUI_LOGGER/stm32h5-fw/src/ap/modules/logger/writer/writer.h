#ifndef WRITER_H_
#define WRITER_H_

#include "ap_def.h"
#include "peri_config.h"
// -- 
// SD 기록 설정 값
#define WRITER_LINE_MAX   (32 + (PERI_SERIAL_MAX_BUFF_LEN * 3))

void writerRequestPause(void);
void writerRequestResume(void);

#endif
