#ifndef CAN_APP_H
#define CAN_APP_H

#include "ap_def.h"

typedef struct
{
    uint32_t id;
    uint8_t  len;
    uint8_t  data[8];
    uint32_t rx_count;
    bool     valid;
} can_frame_t;

void canAppInit(void);
void canAppOnRx(uint8_t ch,const can_msg_t *msg);

const can_frame_t* canAppGetLatest(uint8_t ch);
uint32_t canAppGetSavedNum(uint8_t ch);

void canLogOpen(uint8_t ch);
bool canLogWrite(uint8_t ch, uint32_t id, uint8_t len, uint8_t *data);
void canLogClose(uint8_t ch);

#endif
