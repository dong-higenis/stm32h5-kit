#include "rs485_app.h"
#include "rs485_ui.h"

#define RS485_LOG_BUF_SIZE 256

#if RS485_DEBUG
#define RS485_DBG(fmt, ...) cliPrintf("[RS485] " fmt "\r\n", ##__VA_ARGS__)
#else
#define RS485_DBG(...)
#endif

static FIL  rs485_file[RS485_SEL_MAX_CH];
static bool rs485_open[RS485_SEL_MAX_CH];

static char     rs485_log_buf[RS485_SEL_MAX_CH][RS485_LOG_BUF_SIZE];
static uint16_t rs485_log_len[RS485_SEL_MAX_CH];

/* SD에 이미 있던 로그 개수 */
static uint32_t rs485_saved_base[RS485_SEL_MAX_CH];

/* 이번 세션에서 새로 저장한 로그 개수 */
static uint32_t rs485_saved[RS485_SEL_MAX_CH];

static const uint8_t rs485_hw_ch[RS485_SEL_MAX_CH] =
{
  HW_UART_CH_RS485_2,
  HW_UART_CH_RS485_1,
};

void rs485AppInit(void)
{
  uartOpen(HW_UART_CH_RS485_1, 38400);
  uartOpen(HW_UART_CH_RS485_2, 38400);

  for (int i = 0; i < RS485_SEL_MAX_CH; i++)
  {
    rs485_saved_base[i] = sdGetLogCount("RS485", i);
    rs485_saved[i]      = 0;
  }
}

void rs485AppOnRx(uint8_t ui_ch)
{
  if (ui_ch >= RS485_SEL_MAX_CH)
    return;

  uint8_t hw_ch = rs485_hw_ch[ui_ch];

  while (uartAvailable(hw_ch))
  {
    uint8_t c = uartRead(hw_ch);

    /* UI 실시간 출력 */
    rs485UiPushBytes(ui_ch, &c, 1);

    /* ---------- SD 로그 버퍼 ---------- */
    uint16_t *len = &rs485_log_len[ui_ch];
    char     *buf = rs485_log_buf[ui_ch];

    if (*len < RS485_LOG_BUF_SIZE - 1)
    {
      buf[(*len)++] = (char)c;

      /* 한 줄 끝 -> SD 기록 */
      if (c == '\n')
      {
        buf[*len] = 0;

        if (rs485LogWrite(ui_ch, buf, *len))
        {
          rs485_saved[ui_ch]++; // 이번 세션 카운트 증가
        }

        *len = 0;
      }
    }
    else
    {
      /* 버퍼 가득 찼으면 강제 flush */
      if (rs485LogWrite(ui_ch, buf, *len))
      {
        /* 줄이 아니므로 카운트 증가 안 함 */
      }

      *len = 0;
    }
  }
}

/**
 * @brief UI에서 보는 "총 저장 라인 수"
 */
uint32_t rs485AppGetSavedNum(uint8_t ch)
{
  if (ch >= RS485_SEL_MAX_CH)
    return 0;

  /* SD에 있던 것 + 이번에 쓴 것 */
  return rs485_saved_base[ch] + rs485_saved[ch];
}

void rs485LogOpen(uint8_t ch)
{
  if (ch >= RS485_SEL_MAX_CH)
    return;

  char name[32];
  snprintf(name, sizeof(name), "RS485_CH%d.TXT", ch);

  rs485_open[ch] = sdFileOpen(&rs485_file[ch], name);
}

bool rs485LogWrite(uint8_t ch, const void *buf, uint32_t len)
{
  if (ch >= RS485_SEL_MAX_CH)
    return false;

  if (!rs485_open[ch])
    return false;

  return sdFileWrite(&rs485_file[ch], buf, len);
}

void rs485LogClose(uint8_t ch)
{
  if (ch >= RS485_SEL_MAX_CH)
    return;

  if (rs485_open[ch])
  {
    sdFileClose(&rs485_file[ch]);
    rs485_open[ch] = false;
  }
}
