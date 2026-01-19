#include "ui_draw.h"
#include "ui_def.h"

#define BTN_POS_X  4   // 버튼 초기위치 (x)
#define BTN_POS_Y  14  // 버튼 초기위치 (y)
#define BTN_SPACE  2   // 버튼과 버튼 사이 간격
#define BTN_WIDTH  120 // 버튼의 너비
#define BTN_HEIGHT 14  // 버튼의 높이

extern const peri_detail_t peripherals[];

/**
 * @brief main menu 그리기
 */
void drawMainMenu(uint8_t selected)
  {
    if (!lcdDrawAvailable()) return;

  lcdClearBuffer(black);
  lcdSetFont(LCD_FONT_05x08);

  int scroll_y = selected * (BTN_HEIGHT + BTN_SPACE); // 실제로 내려가야 하는 y값

  lcdPrintf(48, 2 - scroll_y, white, "MENU");

  const char *menu_names[L1_UI_MAX] =
  {  
    "PERIPHERAL",
    "TIME"};

  for (int i = 0; i < L1_UI_MAX; i++)
  {
    int y = BTN_POS_Y + i * (BTN_HEIGHT + BTN_SPACE) - scroll_y; // 논리적인 인덱스와 실제 좌표값을 매핑하여, 스크롤링 구현

    int text_w = getTextWidth(menu_names[i]);
    int text_x = BTN_POS_X + (BTN_WIDTH - text_w) / 2;

    if (i == selected)
    {
      drawSelectedBox(BTN_POS_X, y, BTN_WIDTH, BTN_HEIGHT, menu_names[i]);
    }
    else
    {
      drawNormalBox(BTN_POS_X, y, BTN_WIDTH, BTN_HEIGHT, menu_names[i]);
    }
  }

  lcdRequestDraw();
}

/**
 * @brief peri menu 그리기
 */
void drawProtocolMenu(uint8_t selected)
{
  if (!lcdDrawAvailable()) return;

  lcdClearBuffer(black);
  lcdSetFont(LCD_FONT_05x08);

  int scroll_y = selected * (BTN_HEIGHT + BTN_SPACE);

  lcdPrintf(30, 2 - scroll_y, white, "PERIPHERAL");

  for (int i = 0; i < L2_UI_MAX; i++)
  {
    int y = BTN_POS_Y + i * (BTN_HEIGHT + BTN_SPACE) - scroll_y;

    if (i == selected)
    {
      drawSelectedBox(BTN_POS_X, y, BTN_WIDTH, BTN_HEIGHT, peripherals[i].peri_name);
    }
    else
    {
      drawNormalBox(BTN_POS_X, y, BTN_WIDTH, BTN_HEIGHT, peripherals[i].peri_name);
    }
  }

  lcdRequestDraw();
}

/**
 * @brief peri menu 선택 후 화면 그리기
 */
void drawProtocolDetailMenu(uint8_t selected_peripheral, uint8_t selected_feature) // user가 선택한 peri, 선택한 peri에 대한 기능
{
  const peri_detail_t *p = &peripherals[selected_peripheral];

  if (!lcdDrawAvailable())
  {
    return;
  }

  lcdClearBuffer(black);
  lcdSetFont(LCD_FONT_05x08);

  int scroll_y = selected_feature * (BTN_HEIGHT + BTN_SPACE);

  lcdPrintf(30, 2 - scroll_y, white, p->peri_name);

  for (int i = 0; i < p->button_count; i++)
  {
    int y = BTN_POS_Y + i * (BTN_HEIGHT + BTN_SPACE) - scroll_y;

    if (i == selected_feature)
    {
      drawSelectedBox(BTN_POS_X, y, BTN_WIDTH, BTN_HEIGHT, p->button_list[i]);
    }
    else
    {
      drawNormalBox(BTN_POS_X, y, BTN_WIDTH, BTN_HEIGHT, p->button_list[i]);
    }
  }

  lcdRequestDraw();
}

void drawChannelMenu(uint8_t selected_channel, uint8_t max_ch)
{
  if (!lcdDrawAvailable())
    return;

  lcdClearBuffer(black);
  lcdSetFont(LCD_FONT_05x08);

  int scroll_y = selected_channel * (BTN_HEIGHT + BTN_SPACE);

  lcdPrintf(30, 2 - scroll_y, white, "CHANNEL");

  for (int i = 0; i < max_ch; i++)
  {
    int y = BTN_POS_Y + i * (BTN_HEIGHT + BTN_SPACE) - scroll_y;

    char name[16];
    snprintf(name, sizeof(name), "CH%d", i);

    if (i == selected_channel)
      drawSelectedBox(BTN_POS_X, y, BTN_WIDTH, BTN_HEIGHT, name);
    else
      drawNormalBox(BTN_POS_X, y, BTN_WIDTH, BTN_HEIGHT, name);
  }

  lcdRequestDraw();
}

void drawByteBox(int x, int y, uint8_t value)
{
  char buf[4];

  lcdDrawRect(x, y, 28, 12, white);

  // "XX" 텍스트 중앙 정렬 (기존 getTextWidth 사용)
  int text_w = getTextWidth("FF");         // 5*2 +1*1 = 11픽셀
  int text_x = x + (28 - text_w) / 2;      // (28-11)/2 = 8
  int text_y = (y + (12 - 8) / 2 + 1) - 1; // 폰트 높이 8, 세로 중앙 (y+3)

  snprintf(buf, sizeof(buf), "%02X", value);
  lcdPrintf(text_x, text_y, white, buf);
}

int getTextWidth(const char *str)
{
  int char_w  = 5;                           // 폰트 비트맵 폭
  int spacing = 2;                           // lcdPrintf에서 x += font_width + 1; 형태라면 1

  int len = strlen(str);
  if (len == 0) return 0;

  return len * (char_w + spacing) - spacing; // 마지막 글자 뒤에는 간격 없음
}

void drawSelectedBox(int x, int y, int w, int h, const char *text)
{
  int text_w = getTextWidth(text);
  int text_x = x + (w - text_w) / 2;

  lcdDrawRect(x - 1, y - 1, w + 2, h + 2, white);
  lcdDrawRect(x, y, w, h, black);
  lcdDrawFillRect(x + 1, y + 1, w - 2, h - 2, white);
  lcdPrintf(text_x, y + 3, black, "%s", text);
}

void drawNormalBox(int x, int y, int w, int h, const char *text)
{
  int text_w = getTextWidth(text);
  int text_x = x + (w - text_w) / 2;

  lcdDrawRect(x, y, w, h, white);
  lcdPrintf(text_x, y + 3, white, "%s", text);
}
