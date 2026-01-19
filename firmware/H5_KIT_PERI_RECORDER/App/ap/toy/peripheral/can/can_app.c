#include "can_app.h"
#include "can_ui.h"
#include <string.h>

typedef struct
{
  volatile can_frame_t slot[2];
  volatile uint8_t write_idx;
} can_frame_slot_t;

static can_frame_slot_t can_slot[CAN_SEL_MAX_CH];

uint32_t can_saved[CAN_SEL_MAX_CH];

static FIL  can_file[CAN_SEL_MAX_CH];
static bool can_open[CAN_SEL_MAX_CH];

void canAppInit(void)
{
  memset(can_slot, 0, sizeof(can_slot));

  for (int ch=0; ch < CAN_SEL_MAX_CH; ch++)
  {
    can_saved[ch] = sdGetLogCount("CAN", ch);
  }
}

void canAppOnRx(uint8_t ch, const can_msg_t *msg)
{
  if (ch >= CAN_SEL_MAX_CH) return;
  if (msg == NULL) return;
  if (msg->id == 0x120 || msg->id == 0x121) return;

  uint8_t len = msg->length;
  if (len > 8) len = 8;   // ← 핵심

  can_frame_slot_t *s = &can_slot[ch];
  can_frame_t *f = &s->slot[s->write_idx];

  f->id  = msg->id;
  f->len = len;
  memcpy(f->data, msg->data, len);
  f->rx_count++;
  f->valid = true;

  s->write_idx ^= 1;
}

const can_frame_t* canAppGetLatest(uint8_t ch)
{
  if (ch >= CAN_SEL_MAX_CH) return NULL;

  can_frame_slot_t *s = &can_slot[ch];

  uint8_t read_idx = s->write_idx ^ 1;   // 항상 마지막으로 완성된 슬롯

  return &s->slot[read_idx];
}



uint32_t canAppGetSavedNum(uint8_t ch)
{
  return can_saved[ch];
}

void canLogOpen(uint8_t ch)
{
  if (ch >= CAN_SEL_MAX_CH)
    return;

  char name[32];
  snprintf(name, sizeof(name), "CAN_CH%d.TXT", ch);

  can_open[ch] = sdFileOpen(&can_file[ch], name);
}

bool canLogWrite(uint8_t ch, uint32_t id, uint8_t len, uint8_t *data)
{
  if (ch >= CAN_SEL_MAX_CH)
    return false;

  if (!can_open[ch])
    return false;

  char line[200];
  int pos = snprintf(line, sizeof(line),
                     "%lu\tID:%03X\tLEN:%d\t",
                     millis(), id, len);

  for (int i = 0; i < len; i++)
    pos += snprintf(&line[pos], sizeof(line) - pos, "%02X ", data[i]);

  pos += snprintf(&line[pos], sizeof(line) - pos, "\n");

  return sdFileWrite(&can_file[ch], line, pos);
}

void canLogClose(uint8_t ch)
{
  if (ch >= CAN_SEL_MAX_CH)
    return;

  if (can_open[ch])
  {
    sdFileClose(&can_file[ch]);
    can_open[ch] = false;
  }
}
