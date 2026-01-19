#include "can_ui.h"
#include "can_app.h"
#include "ui.h"

#define CAN_DATA_ROW_1_Y 30
#define CAN_DATA_ROW_2_Y 50

static void drawCanMonitor(void);
static void drawCanLogger(void);

static uint32_t log_keep_ms  = 0;
static uint32_t captured_num = 0;
static bool     is_captured  = false;

extern volatile uint32_t can_saved[CAN_SEL_MAX_CH];

extern const peri_detail_t peripherals[];

void canUiDraw(const char *feature)
{
  if (strcmp(feature, "CAN_MONITOR") == 0)
    drawCanMonitor();
  else if (strcmp(feature, "CAN_LOGGER") == 0)
    drawCanLogger();
}

void canUiOnSelect(const char *feature)
{
  if (strcmp(feature, "CAN_MONITOR") == 0)
  {
    const can_frame_t *frame = canAppGetLatest(uiGet()->ch_index);
    uint8_t            ch    = uiGet()->ch_index;

    if (frame && frame->valid)
    {
      canLogWrite(ch, frame->id, frame->len, frame->data);
      log_keep_ms = millis();
      captured_num++;
      if (ch < CAN_SEL_MAX_CH)
      {
        can_saved[ch]++; // 해당 CAN 채널의 저장 횟수 +1
      }
      is_captured = true;
    }
  }
}

/**
 * @brief STATICS
 */
static void drawCanMonitor(void)
{
  if (!lcdDrawAvailable())
  {
    return;
  }

  lcdClearBuffer(black);
  lcdSetFont(LCD_FONT_05x08);

  const can_frame_t *new_message = canAppGetLatest(uiGet()->ch_index);

  if (new_message && new_message->valid)
  {
    lcdPrintf(2, 0, white, "ID:0x%lX", new_message->id);
    lcdPrintf(2, 17, white, "RX:%ld", new_message->rx_count);

    if (is_captured)
    {
      if (millis() - log_keep_ms < 300)
      {
        lcdDrawFillRect(0, 0, 128, 10, white);
        lcdPrintf(22, 1, black, "CAPTURED:%ld", captured_num);
      }
      else
      {
        is_captured  = false;
        captured_num = 0;
      }
    }

    drawByteBox(0, CAN_DATA_ROW_1_Y, new_message->data[0]);
    drawByteBox(32, CAN_DATA_ROW_1_Y, new_message->data[1]);
    drawByteBox(64, CAN_DATA_ROW_1_Y, new_message->data[2]);
    drawByteBox(96, CAN_DATA_ROW_1_Y, new_message->data[3]);

    drawByteBox(0, CAN_DATA_ROW_2_Y, new_message->data[4]);
    drawByteBox(32, CAN_DATA_ROW_2_Y, new_message->data[5]);
    drawByteBox(64, CAN_DATA_ROW_2_Y, new_message->data[6]);
    drawByteBox(96, CAN_DATA_ROW_2_Y, new_message->data[7]);
  }
  else
  {
    lcdPrintf(20, 16, white, "Waiting CAN");
  }

  lcdRequestDraw();
}

static void drawCanLogger(void)
{
  if (!lcdDrawAvailable())
  {
    return;
  }

  lcdClearBuffer(black);
  lcdSetFont(LCD_FONT_05x08);

  int num_lines = canAppGetSavedNum(uiGet()->ch_index); // 저장된 canlog 줄 수

  lcdPrintf(32, 0, white, "CAN LOGGER");

  if (num_lines <= 0)
  {
    lcdPrintf(30, 30, white, "No Data");
  }
  else
  {
    lcdPrintf(24, 20, white, "Saved Frames");
    lcdPrintf(54, 34, white, "%d", num_lines);
  }

  lcdRequestDraw();
}
