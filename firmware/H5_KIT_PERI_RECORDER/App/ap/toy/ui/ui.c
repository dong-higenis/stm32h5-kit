#include "ui.h"
#include "ui_def.h"

#define BTN_NEXT   0x120
#define BTN_SELECT 0x121

static void uiNext(void);
static void uiSelect(void);

static ui_context_t ui;

static const char *can_detail_menu[] =
{
  "CAN_MONITOR",
  "CAN_LOGGER",
  "RETURN"};

static const char *rs485_detail_menu[] =
{
  "RS485_MONITOR",
  "RS485_LOGGER",
  "RETURN"};

static const char *uart_detail_menu[] =
{
  "UART_MONITOR",
  "UART_LOGGER",
  "RETURN"};

const peri_detail_t peripherals[] =
{
  {"CAN",    can_detail_menu,   3, CAN_SEL_MAX_CH  },
  {"RS485",  rs485_detail_menu, 3, RS485_SEL_MAX_CH},
  {"RS232",  NULL,              0, RS232_SEL_MAX_CH},
  {"UART",   uart_detail_menu,  3, UART_SEL_MAX_CH },
  {"LIN",    NULL,              0, LIN_SEL_MAX_CH  },
  {"RETURN", NULL,              0, 0               }
};

void uiInit(void)
{
  ui.main_layer = UI_MAIN_LAYER_MENU;
  ui.l1_index   = 0;
  ui.l2_index   = 0;
  ui.l3_index   = 0;
  ui.ch_index   = 0;
}

void uiOnButton(uint32_t id)
{
  if (id == BTN_NEXT)        // can 0x120
  {
    uiNext();
  }
  else if (id == BTN_SELECT) // can 0x121
  {
    uiSelect();
  }
}

void uiDraw(void)
{
  const ui_context_t *current_ui = uiGet();

  switch (current_ui->main_layer)
  {
    case UI_MAIN_LAYER_MENU:
      drawMainMenu(current_ui->l1_index);
      break;

    case UI_MAIN_LAYER_TIME:
      drawTimeScreen();
      break;

    case UI_MAIN_LAYER_PROTOCOL:
      drawProtocolMenu(current_ui->l2_index);
      break;

    case UI_MAIN_LAYER_PROTOCOL_DETAIL:
      drawProtocolDetailMenu(current_ui->l2_index, current_ui->l3_index);
      break;

    case UI_MAIN_LAYER_PROTOCOL_CHANNEL_SELECT:
      {
        const peri_detail_t *p = &peripherals[current_ui->l2_index];
        drawChannelMenu(current_ui->ch_index, p->ch_max);
        break;
      }

    case UI_MAIN_LYAER_PROTOCOL_FEATURE:
      {
        const peri_detail_t *p       = &peripherals[current_ui->l2_index];
        const char          *feature = p->button_list[current_ui->l3_index];

        if (strcmp(p->peri_name, "CAN") == 0)
        {
          canUiDraw(feature);
        }
        else if (strcmp(p->peri_name, "RS485") == 0)
        {
          rs485UiDraw(feature);
        }
        else if (strcmp(p->peri_name, "UART") == 0)
        {
          uartUiDraw(feature);
        }
      }
      break;
  }
}

const ui_context_t *uiGet(void)
{
  return &ui;
}

/**
 * @brief static 함수
 */

// 0x120
static void uiNext(void)
{
  switch (ui.main_layer)
  {
    case UI_MAIN_LAYER_MENU:
      ui.l1_index = (ui.l1_index + 1) % L1_UI_MAX;
      break;

    case UI_MAIN_LAYER_TIME:
      ui.main_layer = UI_MAIN_LAYER_MENU; // back to menu
      break;

    case UI_MAIN_LAYER_PROTOCOL:
      ui.l2_index = (ui.l2_index + 1) % L2_UI_MAX;
      break;

    case UI_MAIN_LAYER_PROTOCOL_DETAIL:
      {
        const peri_detail_t *peri = &peripherals[ui.l2_index];
        if (peri->button_count > 0)
        {
          ui.l3_index = (ui.l3_index + 1) % peri->button_count;
        }
        break;
      }

    case UI_MAIN_LAYER_PROTOCOL_CHANNEL_SELECT:
      {
        const peri_detail_t *p = &peripherals[ui.l2_index];
        ui.ch_index            = (ui.ch_index + 1) % p->ch_max;
        break;
      }

    case UI_MAIN_LYAER_PROTOCOL_FEATURE:
      {
        const peri_detail_t *p      = &peripherals[ui.l2_index];
        uint8_t              ch = ui.ch_index;

        ui.main_layer = UI_MAIN_LAYER_PROTOCOL_DETAIL;

        if (strcmp(p->peri_name, "UART") == 0)
        {
          uartLogClose(ch);
        }
        else if (strcmp(p->peri_name, "RS485") == 0)
        {
          rs485LogClose(ch);
        }
        else if (strcmp(p->peri_name, "CAN") == 0)
        {
          canLogClose(ch);
        }
        break;
      }

      break;

    default:
      break;
  }
}

// 0x121
static void uiSelect(void)
{
  switch (ui.main_layer)
  {
    case UI_MAIN_LAYER_MENU:
      if (ui.l1_index == L1_PERIPHERAL) // menu -> peri버튼 누를시
      {
        ui.l2_index   = 0;
        ui.main_layer = UI_MAIN_LAYER_PROTOCOL;
      }
      else if (ui.l1_index == L1_TIME)  // menu -> time버튼 누를시
      {
        ui.main_layer = UI_MAIN_LAYER_TIME;
      }
      break;

    case UI_MAIN_LAYER_PROTOCOL:
      if (ui.l2_index == L2_RETURN)
      {
        ui.main_layer = UI_MAIN_LAYER_MENU;
      }
      else
      {
        ui.l3_index   = 0;                         // 다른 peri선택시 이전 기록을 지우기 위해 초기화
        ui.main_layer = UI_MAIN_LAYER_PROTOCOL_DETAIL;
      }
      break;

    case UI_MAIN_LAYER_PROTOCOL_DETAIL:            // peri별 선택 화면
      {
        const peri_detail_t *peri = &peripherals[ui.l2_index];

        if (ui.l3_index == peri->button_count - 1) // RETURN 버튼
        {
          ui.main_layer = UI_MAIN_LAYER_PROTOCOL;
        }
        else
        {
          ui.ch_index   = 0; // 채널 인덱스 초기화
          ui.main_layer = UI_MAIN_LAYER_PROTOCOL_CHANNEL_SELECT;
        }
        break;
      }

    case UI_MAIN_LAYER_PROTOCOL_CHANNEL_SELECT:
      {
        const peri_detail_t *p  = &peripherals[ui.l2_index]; // user가 선택한 통신
        uint8_t              ch = ui.ch_index;               // user가 선택한 채널

        ui.main_layer = UI_MAIN_LYAER_PROTOCOL_FEATURE;
        // 여기서 부터 peri별로 logFile open
        if (strcmp(p->peri_name, "RS485") == 0)
        {
          rs485LogOpen(ch);
        }
        else if (strcmp(p->peri_name, "UART") == 0)
        {
          uartLogOpen(ch);
        }
        else if (strcmp(p->peri_name, "CAN") == 0)
        {
          canLogOpen(ch);
        }
        break;
      }


    case UI_MAIN_LYAER_PROTOCOL_FEATURE:
      {
        const peri_detail_t *p       = &peripherals[ui.l2_index];
        const char          *feature = p->button_list[ui.l3_index];

        if (strcmp(p->peri_name, "CAN") == 0)
        {
          canUiOnSelect(feature);
        }
        // uart는 그냥 다 기록
        break;
      }
  }
}