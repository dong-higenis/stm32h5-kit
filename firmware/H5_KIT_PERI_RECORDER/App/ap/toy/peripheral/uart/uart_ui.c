#include "uart_ui.h"
#include "uart_app.h"
#include "ui.h"

#define UART_UI_BUF 64

static uint8_t  ui_buf[UART_SEL_MAX_CH][UART_UI_BUF];
static uint16_t ui_len[UART_SEL_MAX_CH];

static void drawUartMonitor(void);
static void drawUartLogger(void);

void uartUiDraw(const char *feature)
{
  if (strcmp(feature, "UART_MONITOR") == 0)
  {
    drawUartMonitor();
  }
  else if (strcmp(feature, "UART_LOGGER") == 0)
  {
    drawUartLogger();
  }
}

void uartUiOnSelect(const char *feature)
{
  // UART는 클릭으로 할 동작 없음 (지금은)
}

void uartUiPushBytes(uint8_t ch, uint8_t *data, uint16_t len)
{
  if (ch >= UART_SEL_MAX_CH) return;

  uint16_t *l = &ui_len[ch];

  for (uint16_t i = 0; i < len; i++)
  {
    if (*l < UART_UI_BUF)
    {
      ui_buf[ch][(*l)++] = data[i];
    }
    else
    {
      // scroll
      memmove(&ui_buf[ch][0], &ui_buf[ch][1], UART_UI_BUF - 1);
      ui_buf[ch][UART_UI_BUF - 1] = data[i];
    }
  }
}

static void drawUartMonitor(void)
{
  if (!lcdDrawAvailable())
    return;

  lcdClearBuffer(black);
  lcdSetFont(LCD_FONT_05x08);

  uint8_t ch = uiGet()->ch_index;
  
  static uint32_t count = 0;
  static uint32_t pre_time = 0;

  if (millis() - pre_time > 100)
  {
    pre_time = millis();
    count = uartAppGetSavedNum(ch);
  }

  lcdPrintf(2, 0, white, "UART CH%d", ch);
  lcdPrintf(2, 12, white, "Saved: %lu", count);

  lcdPrintf(2, 30, white, "Receiving...");
  lcdPrintf(2, 40, white, "rx:");

  // "rx:" 다음 위치부터 출력
  int x = 2 + getTextWidth("rx:");
  const int y = 40;

  for (int i = 0; i < ui_len[ch]; i++)
  {
    uint8_t b = ui_buf[ch][i];

    // 출력 가능한 ASCII만, 나머지는 '.'
    char c = (b >= 32 && b <= 126) ? (char)b : '.';

    // 화면 끝에 닿으면 rx 영역만 clear 후 다시 시작
    if (x > (HW_LCD_WIDTH - 6))   // 6 = 글자 폭(5) + spacing(1)
    {
      // rx 뒤쪽만 지움
      lcdDrawFillRect(2 + getTextWidth("rx:"), y, HW_LCD_WIDTH, 10, black);
      x = 2 + getTextWidth("rx:");
    }

    lcdPrintf(x, y, white, "%c", c);
    x += 6;
  }

  lcdRequestDraw();
}


static void drawUartLogger(void)
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
    count = uartAppGetSavedNum(ch);
  }

  lcdPrintf(20, 0, white, "UART LOGGER");
  lcdPrintf(2, 20, white, "CH%d Saved Bytes:", ch);
  lcdPrintf(40, 36, white, "%lu", count);

  lcdRequestDraw();
}
