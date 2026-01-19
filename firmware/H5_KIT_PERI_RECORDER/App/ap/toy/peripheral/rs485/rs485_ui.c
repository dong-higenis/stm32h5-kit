#include "rs485_ui.h"
#include "rs485_app.h"
#include "ui.h"

#define RS485_UI_BUF 64

static uint8_t  ui_buf[RS485_SEL_MAX_CH][RS485_UI_BUF];
static uint16_t ui_len[RS485_SEL_MAX_CH];

static void drawRs485Monitor(void);
static void drawRs485Logger(void);

void rs485UiDraw(const char *feature)
{
  if (strcmp(feature, "RS485_MONITOR") == 0)
  {
    drawRs485Monitor();
  }
  else if (strcmp(feature, "RS485_LOGGER") == 0)
  {
    drawRs485Logger();
  }
}

void rs485UiPushBytes(uint8_t ch, uint8_t *data, uint16_t len)
{
  if (ch >= RS485_SEL_MAX_CH) return;

  uint16_t *l = &ui_len[ch];

  for (uint16_t i = 0; i < len; i++)
  {
    if (*l < RS485_UI_BUF)
    {
      ui_buf[ch][(*l)++] = data[i];
    }
    else
    {
      // scroll
      memmove(&ui_buf[ch][0], &ui_buf[ch][1], RS485_UI_BUF - 1);
      ui_buf[ch][RS485_UI_BUF - 1] = data[i];
    }
  }
}

static void drawRs485Monitor(void)
{
  if (!lcdDrawAvailable())
    return;

  lcdClearBuffer(black);
  lcdSetFont(LCD_FONT_05x08);

  uint8_t  ch    = uiGet()->ch_index;
  static uint32_t count = 0;
  static uint32_t pre_time = 0;

  if (millis() - pre_time > 100)
  {
    pre_time = millis();
    count = rs485AppGetSavedNum(ch);
  }

  lcdPrintf(2, 0, white, "RS485 CH%d", ch);
  lcdPrintf(2, 12, white, "Saved: %lu", count);

  lcdPrintf(2, 30, white, "Receiving...");
  lcdPrintf(2, 40, white, "rx:");

  int       x = 2 + getTextWidth("rx:");
  const int y = 40;

  for (int i = 0; i < ui_len[ch]; i++)
  {
    uint8_t b = ui_buf[ch][i];
    char    c = (b >= 32 && b <= 126) ? (char)b : '.';

    if (x > (LCD_WIDTH - 6))
    {
      lcdDrawFillRect(2 + getTextWidth("rx:"), y, LCD_WIDTH, 10, black);
      x = 2 + getTextWidth("rx:");
    }

    lcdPrintf(x, y, white, "%c", c);
    x += 6;
  }

  lcdRequestDraw();
}

static void drawRs485Logger(void)
{
  if (!lcdDrawAvailable())
    return;

  lcdClearBuffer(black);
  lcdSetFont(LCD_FONT_05x08);

  uint8_t  ch    = uiGet()->ch_index;
  
  static uint32_t count = 0;
  static uint32_t pre_time = 0;

  if (millis() - pre_time > 100)
  {
    pre_time = millis();
    count = rs485AppGetSavedNum(ch);
  }

  lcdPrintf(20, 0, white, "RS485 LOGGER");
  lcdPrintf(2, 20, white, "CH%d Saved Bytes:", ch);
  lcdPrintf(40, 36, white, "%lu", count);

  lcdRequestDraw();
}
