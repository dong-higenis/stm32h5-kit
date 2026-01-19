#include "uart_app.h"
#include "uart_ui.h"

#define UART_LOG_BUF_SIZE 256

static FIL  uart_file[UART_SEL_MAX_CH];
static bool uart_open[UART_SEL_MAX_CH];

static char     uart_log_buf[UART_SEL_MAX_CH][UART_LOG_BUF_SIZE];
static uint16_t uart_log_len[UART_SEL_MAX_CH];

/* SD에 이미 존재하던 라인 수 */
static uint32_t uart_saved_base[UART_SEL_MAX_CH];

/* 이번 실행에서 새로 저장한 라인 수 */
static uint32_t uart_saved[UART_SEL_MAX_CH];

static const uint8_t uart_hw_ch[UART_SEL_MAX_CH] =
{
  HW_UART_CH_UART_1,
  HW_UART_CH_UART_2,
};

void uartAppInit(void)
{
  uartOpen(HW_UART_CH_UART_1, 115200);
  uartOpen(HW_UART_CH_UART_2, 115200);

  for (int i = 0; i < UART_SEL_MAX_CH; i++)
  {
    uart_saved_base[i] = sdGetLogCount("UART", i);
    uart_saved[i]      = 0;
    uart_log_len[i]    = 0;
    uart_open[i]      = false;
  }
}

/**
 * @note 실제 물리 채널과 UI 채널은 1씩 차이남
 */
void uartAppOnRx(uint8_t ui_ch)
{
  if (ui_ch >= UART_SEL_MAX_CH)
    return;

  uint8_t hw_ch = uart_hw_ch[ui_ch];

  while (uartAvailable(hw_ch))
  {
    uint8_t c = uartRead(hw_ch);

    /* UI 실시간 표시 */
    uartUiPushBytes(ui_ch, &c, 1);

    /* ---------- SD 로그 버퍼 ---------- */
    uint16_t *len = &uart_log_len[ui_ch];
    char     *buf = uart_log_buf[ui_ch];

    if (*len < UART_LOG_BUF_SIZE - 2)
    {
      buf[(*len)++] = (char)c;

      /* 줄 끝 처리 */
      if (c == '\n' || c == '\r')
      {
        /* CR/LF 통합 → LF */
        buf[*len - 1] = '\n';

        if (uartLogWrite(ui_ch, buf, *len))
        {
          uart_saved[ui_ch]++;   // 이번 세션 라인 수 증가
        }

        *len = 0;
      }
    }
    else
    {
      /* 버퍼 가득 → 강제 flush (라인 아님) */
      uartLogWrite(ui_ch, buf, *len);
      *len = 0;
    }
  }
}

/**
 * @brief UI에서 보는 총 저장 라인 수
 */
uint32_t uartAppGetSavedNum(uint8_t ch)
{
  if (ch >= UART_SEL_MAX_CH)
    return 0;

  return uart_saved_base[ch] + uart_saved[ch];
}

void uartLogOpen(uint8_t ch)
{
  if (ch >= UART_SEL_MAX_CH)
    return;

  char name[32];
  snprintf(name, sizeof(name), "UART_CH%d.TXT", ch);

  uart_open[ch] = sdFileOpen(&uart_file[ch], name);
}

bool uartLogWrite(uint8_t ch, const void *buf, uint32_t len)
{
  if (ch >= UART_SEL_MAX_CH)
    return false;

  if (!uart_open[ch])
    return false;

  return sdFileWrite(&uart_file[ch], buf, len);
}

void uartLogClose(uint8_t ch)
{
  if (ch >= UART_SEL_MAX_CH)
    return;

  if (uart_open[ch])
  {
    sdFileClose(&uart_file[ch]);
    uart_open[ch] = false;
  }
}
