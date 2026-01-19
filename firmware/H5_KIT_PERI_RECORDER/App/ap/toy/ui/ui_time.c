#include "ui_time.h"
#include "ui.h"

void drawTimeScreen(void)
{
  if (!lcdDrawAvailable())
    return;

  lcdClearBuffer(black);
  lcdSetFont(LCD_FONT_05x08);

  uint32_t sec = millis() / 1000;
  uint32_t min = sec / 60;
  uint32_t hour = min / 60;

  sec %= 60;
  min %= 60;

  lcdPrintf(28, 0, white, "SYSTEM TIME");
  lcdPrintf(30, 24, white, "%02lu:%02lu:%02lu", hour, min, sec);
  lcdPrintf(8, 52, white, "0x120 : BACK");

  lcdRequestDraw();
}

