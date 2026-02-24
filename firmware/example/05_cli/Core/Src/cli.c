#include "cli.h"
#include "usart.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * @brief 해당 함수코드는 유튜버 바람님의 코드를 참조하여 만들었습니다.
 */

/**
 * @brief ANSI 표준 기반 각종 키 입력값 정의
 */
#define CLI_KEY_BACK  0x08 // back space 키
#define CLI_KEY_DEL   0x7F // delete 키
#define CLI_KEY_ENTER 0x0D // enter 키
#define CLI_KEY_ESC   0x1B // esc 키
#define CLI_KEY_LEFT  0x44 // left 키
#define CLI_KEY_RIGHT 0x43 // right 키
#define CLI_KEY_UP    0x41 // up 키
#define CLI_KEY_DOWN  0x42 // down 키

/**
 * @brief ANSI 표준에서 화살표 키는 첫 1바이트에 ESC문자에 대한 ANSI 입력값을 먼저 전송한 후
 *        2바이트 이후에 파싱된다.
 *        UART는 1바이트씩 수신되기 때문에,
 *        4단계 state로써 현재 몇번째 바이트를 수신했는지 기록하기 위한 state이다.
 *
 *        IDLE에서 ESC문자 코드가 들어온다. state = ESC1 -> state머신에 진입한다. state = ESC2
 *        다음 루프에서 state 머신 내 case : ESC2 로 진입한다. (두번째 바이트 수신 상태),state = ESC3
 *        그리고 마지막 루프에서  case : ESC3 로 진입한다. 마지막 바이트 수신 완료. 이제 적절한 처리를 한 뒤 IDLE상태로 되돌린다.
 */
enum
{
  CLI_RX_IDLE,
  CLI_RX_ESC1,
  CLI_RX_ESC2,
  CLI_RX_ESC3,
};

extern UART_HandleTypeDef huart1; // STLink가 점유하는 포트의 uart 핸들러

typedef struct
{
  char cmd_str[CLI_CMD_NAME_MAX]; // 명령어로 쓰일 문자열
  void (*cmd_func)(cli_args_t *); // 명령어를 받았을때 호출할 함수
} cli_cmd_t;

typedef struct
{
  uint8_t buf[CLI_LINE_BUF_MAX];  // 명령어 및 인자를 모두 포함한 문자열 (예: led on\n)
  uint8_t count;                  // 위 buf의 문자 갯수 (입력마다 strlen()을 호출 할 필요 없이 매번 기록)
  uint8_t cursor;                 // 커서의 현재 위치
} cli_line_t;

typedef struct
{
  uint8_t  state;
  char     print_buffer[256];            // printf를 위한 출력 버퍼
  uint16_t argc;                         // 파싱된 명령어 개수
  char    *argv[CLI_ARGS_MAX];           // 파싱된 각 명령어의 시작 주소

  cli_line_t line;                       // 사용자가 지금 편집 중인 입력 줄 커서이동, 삽입 삭제등을 여기서 관리
  cli_line_t line_buf[CLI_LINE_HIS_MAX]; // 명령어 히스토리 저장소 (이전에 입력한 명령어)

  int8_t  hist_line_i;                   // 현재 보고 있는 히스토리 인덱스
  uint8_t hist_line_last;                // 가장 최근에 저장된 히스토리 위치
  uint8_t hist_line_count;               // 현재 저장된 히스토리 개수
  bool    hist_line_new;                 // 새 입력 줄인지 여부

  uint16_t   cmd_count;                  // 현재 등록된 명령 개수
  cli_cmd_t  cmd_list[CLI_CMD_LIST_MAX]; // 등록된 명령어들: string -> 함수 매핑 테이블
  cli_args_t cmd_args;                   // 명령 실행 시 전달되는 인자
} cli_t;

static cli_t cli_node;

static void cliLineClean(cli_t *p_cli);
static void cliLineAdd(cli_t *p_cli);
static void cliLineChange(cli_t *p_cli, bool key_up);
static void cliToUpper(char *str);
static bool cliRunCmd(cli_t *p_cli);
static bool cliParseArgs(cli_t *p_cli);
static void cliShowPrompt(cli_t *p_cli);

static int32_t cliArgsGetData(uint8_t index);
static float   cliArgsGetFloat(uint8_t index);
static char   *cliArgsGetStr(uint8_t index);
static bool    cliArgsIsStr(uint8_t index, const char *p_str);

void cliShowList(cli_args_t *args);

bool cliInit(void)
{
  cli_node.state           = CLI_RX_IDLE;
  cli_node.hist_line_i     = 0;
  cli_node.hist_line_last  = 0;
  cli_node.hist_line_count = 0;
  cli_node.hist_line_new   = false;

  cli_node.cmd_args.getData  = cliArgsGetData;
  cli_node.cmd_args.getFloat = cliArgsGetFloat;
  cli_node.cmd_args.getStr   = cliArgsGetStr;
  cli_node.cmd_args.isStr    = cliArgsIsStr;

  cliLineClean(&cli_node);

  cliAdd("help", cliShowList); // help 라는 명령어를 추가 하고 cliShowList()함수와 매핑한다.

  return true;
}

/**
 * @brief 사용자가 입력 했던 문자를 모두 지운다.
 */
void cliLineClean(cli_t *p_cli)
{
  p_cli->line.count  = 0;
  p_cli->line.cursor = 0;
  p_cli->line.buf[0] = 0;
}

/**
 * @brief 프롬프트를 설정한다. 항상 cli# 가 출력된다.
 */
void cliShowPrompt(cli_t *p_cli)
{
  cliPrintf("\r\ncli# ");
}

/**
 * @brief 사용자가 입력 했던 문자열(명령어와 인자포함)을 history에 기록한다.
 *        즉, Enter를 누르면 호출됨
 */
void cliLineAdd(cli_t *p_cli)
{
  p_cli->line_buf[p_cli->hist_line_last] = p_cli->line;

  if (p_cli->hist_line_count < CLI_LINE_HIS_MAX)
  {
    p_cli->hist_line_count++;
  }

  p_cli->hist_line_i    = p_cli->hist_line_last;
  p_cli->hist_line_last = (p_cli->hist_line_last + 1) % CLI_LINE_HIS_MAX;
  p_cli->hist_line_new  = true;
}

/**
 * @brief 이전·다음 명령을 불러와 현재 입력 줄을 교체하는 함수, ↑ / ↓를 입력하면 히스토리를 불러온다.
 */
void cliLineChange(cli_t *p_cli, bool key_up)
{
  uint8_t     change_i;
  cli_line_t *line = &p_cli->line;

  if (p_cli->hist_line_count == 0)
    return;

  // 현재 줄 지우기
  if (line->cursor > 0)
  {
    cliPrintf("\x1B[%dD", line->cursor); // 커서를 맨 앞으로
  }
  if (line->count > 0)
  {
    cliPrintf("\x1B[%dP", line->count);  // 모든 문자 삭제
  }

  // 히스토리 인덱스 변경
  if (key_up == true)
  {
    if (p_cli->hist_line_new == true)
    {
      p_cli->hist_line_i = p_cli->hist_line_last;
    }
    p_cli->hist_line_i = (p_cli->hist_line_i + p_cli->hist_line_count - 1) % p_cli->hist_line_count;
    change_i           = p_cli->hist_line_i;
  }
  else
  {
    p_cli->hist_line_i = (p_cli->hist_line_i + 1) % p_cli->hist_line_count;
    change_i           = p_cli->hist_line_i;
  }

  p_cli->line        = p_cli->line_buf[change_i];
  p_cli->line.cursor = p_cli->line.count;

  p_cli->hist_line_new = false;

  // 새 줄 출력
  cliPrintf("%s", p_cli->line.buf);
}

/**
 * @brief 핵심 함수: UART로 받은 1바이트 처리
 *        HAL_UART_RxCpltCallback에서 호출됨
 */
void cliProcess(uint8_t rx_data)
{
  cli_t      *p_cli = &cli_node;    // 전역 cli context (상태/명령테이블 등) 포인터
  cli_line_t *line  = &p_cli->line; // 현재 편집 중인 입력 라인(버퍼/커서/길이) 포인터
  uint8_t     tx_buf[8];            // ANSI 시퀀스 송신용 임시 버퍼, ESC는 3바이트 이므로 3바이트 만큼 선언

  if (p_cli->state == CLI_RX_IDLE)  // 모든 입력값에 대해 별도 분기
  {
    switch (rx_data)
    {
      // Enter: 명령어 실행
      case CLI_KEY_ENTER:
        if (line->count > 0)
        {
          cliLineAdd(p_cli);        // 입력했던 문자열 history에 저장
          cliRunCmd(p_cli);         // 명령어 실제 실행
        }

        line->count  = 0;           // 사용자가 입력했던 문자열 및 커서 초기화
        line->cursor = 0;
        line->buf[0] = 0;
        cliShowPrompt(p_cli);       // 다음줄에 cli# 출력
        break;

      case CLI_KEY_ESC:             // ESC는 2바이트를 더 받아야 하기 때문에,
        p_cli->state = CLI_RX_ESC1; // 별도의 상태로 빠진다.
        break;

      case CLI_KEY_DEL:
        // Delete 키: 커서 위치의 문자 삭제
        if (line->cursor < line->count)
        {
          uint8_t mov_len = line->count - line->cursor;
          // 커서 뒤의 문자들을 한 칸씩 앞으로 이동
          for (int i = 1; i < mov_len; i++)
          {
            line->buf[line->cursor + i - 1] = line->buf[line->cursor + i];
          }

          line->count--;
          line->buf[line->count] = 0;

          cliPrintf("\x1B[1P"); // 한 글자 삭제
        }
        break;

      case CLI_KEY_BACK:
        // Backspace: 커서 앞 문자 삭제
        // 예: "HELLO|" (커서가 끝) -> Backspace -> "HELL|"
        // "HE|LLO" (커서가 중간) -> Backspace -> "H|LLO"
        if (line->count > 0 && line->cursor > 0)
        {
          if (line->cursor == line->count)
          {
            // 커서가 맨 끝에 있을 때
            line->count--;
            line->buf[line->count] = 0;
          }
          else
          {
            // 커서가 중간에 있을 때
            uint8_t mov_len = line->count - line->cursor;

            for (int i = 0; i < mov_len; i++)
            {
              line->buf[line->cursor + i - 1] = line->buf[line->cursor + i]; // 입력된 문자열을 한칸 앞당긴다.
            }

            line->count--;
            line->buf[line->count] = 0;
          }

          line->cursor--;
          cliPrintf("\b \b\x1B[1P"); // 백스페이스 + 공백 + 백스페이스 -> 한 글자 삭제
        }
        break;

      default:
        // 일반 문자 입력 (a~z, 0~9 등)
        if ((line->count + 1) < CLI_LINE_BUF_MAX)
        {
          if (line->cursor == line->count)
          {
            // 커서가 맨 끝
            HAL_UART_Transmit(&huart1, &rx_data, 1, 10);

            line->buf[line->cursor] = rx_data;
            line->count++;
            line->cursor++;
            line->buf[line->count] = 0;
          }
          else
          {
            // 커서가 중간
            uint8_t mov_len = line->count - line->cursor;

            for (int i = 0; i < mov_len; i++)
            {
              line->buf[line->count - i] = line->buf[line->count - i - 1];
            }

            line->buf[line->cursor] = rx_data;
            line->count++;
            line->cursor++;
            line->buf[line->count] = 0;

            cliPrintf("\x1B[4h%c\x1B[4l", rx_data); // Insert 모드로 문자 삽입
          }
        }
        break;
    }
  }

  /**
   * @brief ESC 시퀀스 처리 (화살표 키의 첫 바이트가 ESC)
   *        화살표 키는 3바이트로 전송됨:
   *        - 위: ESC(0x1B) + [(0x5B) + A(0x41)
   *        - 아래: ESC + [ + B
   *        - 왼쪽: ESC + [ + D
   *        - 오른쪽: ESC + [ + C
   */
  switch (p_cli->state)
  {
    case CLI_RX_ESC1: // 첫번째 바이트 받고
      p_cli->state = CLI_RX_ESC2;
      break;
    case CLI_RX_ESC2:
      p_cli->state = CLI_RX_ESC3; // 두번째 바이트를 받은 후
      break;

    case CLI_RX_ESC3: // 마지막 바이트를 받았을때 처리
      p_cli->state = CLI_RX_IDLE;

      if (rx_data == CLI_KEY_LEFT)
      {
        // 왼쪽 화살표
        if (line->cursor > 0)
        {
          line->cursor--;
          tx_buf[0] = 0x1B;                          // esc
          tx_buf[1] = 0x5B;                          // 중간값
          tx_buf[2] = rx_data;                       // left 화살표
          HAL_UART_Transmit(&huart1, tx_buf, 3, 10); // 위 배열을 출력
        }
      }
      else if (rx_data == CLI_KEY_RIGHT)
      {
        // 오른쪽 화살표
        if (line->cursor < line->count)
        {
          line->cursor++;
          tx_buf[0] = 0x1B;
          tx_buf[1] = 0x5B;
          tx_buf[2] = rx_data;
          HAL_UART_Transmit(&huart1, tx_buf, 3, 10);
        }
      }
      else if (rx_data == CLI_KEY_UP)
      {
        // 위쪽 화살표: 이전 명령어
        cliLineChange(p_cli, true);
      }
      else if (rx_data == CLI_KEY_DOWN)
      {
        // 아래쪽 화살표: 다음 명령어
        cliLineChange(p_cli, false);
      }
      break;
  }
}

/**
 * @brief 실제 들어온 명령어를 실행한다.
 */
bool cliRunCmd(cli_t *p_cli)
{
  if (cliParseArgs(p_cli) == true)
  {
    cliPrintf("\r\n");

    cliToUpper(p_cli->argv[0]); // 명령어를 대문자로 변환 (대소문자 구분 안함)

    // 등록된 명령어 리스트에서 검색
    for (int i = 0; i < p_cli->cmd_count; i++)
    {
      // 명령어 찾음: 실행
      if (strcmp(p_cli->argv[0], p_cli->cmd_list[i].cmd_str) == 0)
      {
        p_cli->cmd_args.argc = p_cli->argc - 1;        // 첫 인자(명령어)는 제외
        p_cli->cmd_args.argv = &p_cli->argv[1];        // 인자만 전달
        p_cli->cmd_list[i].cmd_func(&p_cli->cmd_args); // 매핑된 함수가 인자를 가져간다.
        return true;
      }
    }
    cliPrintf("Unknown command: %s\r\n", p_cli->argv[0]); // 못찾은 경우, 에러메시지 출력
  }

  return false;
}

/**
 * @brief 입력 문자열을 공백으로 분리해서 명령어와 인자로 나눔
 * 예: "led on" -> argv[0]="led", argv[1]="on"
 */
bool cliParseArgs(cli_t *p_cli)
{
  bool ret = false;
  char *tok;
  char *next_ptr;
  uint16_t argc = 0;
  static const char *delim = " \f\n\r\t\v";
  char *cmdline;
  char **argv;

  p_cli->argc = 0;

  cmdline = (char *)p_cli->line.buf;
  argv    = p_cli->argv;

  argv[argc] = NULL;

  for (tok = strtok_r(cmdline, delim, &next_ptr); tok; tok = strtok_r(NULL, delim, &next_ptr))
  {
    argv[argc++] = tok;
  }

  p_cli->argc = argc;

  if (argc > 0)
  {
    ret = true;
  }

  return ret;
}

/**
 * @brief 일반 printf처럼 사용 할 수 있게 wrapping 해둔 것
 */
void cliPrintf(const char *fmt, ...)
{
  va_list arg;
  va_start(arg, fmt);
  int32_t len;
  cli_t  *p_cli = &cli_node;

  len = vsnprintf(p_cli->print_buffer, sizeof(p_cli->print_buffer), fmt, arg);
  va_end(arg);

  if (len > 0)
  {
    HAL_UART_Transmit(&huart1, (uint8_t *)p_cli->print_buffer, len, 100); 
  }
}

/**
 * @brief 소문자 -> 대문자
 */
void cliToUpper(char *str)
{
  for (uint16_t i = 0; i < CLI_CMD_NAME_MAX && str[i] != 0; i++)
  {
    if (str[i] >= 'a' && str[i] <= 'z')
    {
      str[i] = str[i] - 'a' + 'A';
    }
  }
}

/**
 * @brief 인자를 정수로 변환
 *        예: args->getData(0) -> "100" 입력 시 100 반환
 */
int32_t cliArgsGetData(uint8_t index)
{
  cli_t *p_cli = &cli_node;

  if (index >= p_cli->cmd_args.argc)
    return 0;

  return (int32_t)strtol(p_cli->cmd_args.argv[index], NULL, 0);
}

float cliArgsGetFloat(uint8_t index)
{
  cli_t *p_cli = &cli_node;

  if (index >= p_cli->cmd_args.argc)
    return 0.0f;

  return strtof(p_cli->cmd_args.argv[index], NULL);
}

char *cliArgsGetStr(uint8_t index)
{
  cli_t *p_cli = &cli_node;

  if (index >= p_cli->cmd_args.argc)
    return NULL;

  return p_cli->cmd_args.argv[index];
}

bool cliArgsIsStr(uint8_t index, const char *p_str)
{
  cli_t *p_cli = &cli_node;

  if (index >= p_cli->cmd_args.argc)
    return false;

  return (strcmp(p_str, p_cli->cmd_args.argv[index]) == 0);
}

/**
 * @brief 새 명령어 등록
 *        예: cliAdd("led", cliLedCmd);
 */
bool cliAdd(const char *cmd_str, void (*p_func)(cli_args_t *))
{
  cli_t *p_cli = &cli_node;

  if (p_cli->cmd_count >= CLI_CMD_LIST_MAX)
    return false;

  uint16_t index = p_cli->cmd_count;

  strncpy(p_cli->cmd_list[index].cmd_str, cmd_str, CLI_CMD_NAME_MAX - 1);
  p_cli->cmd_list[index].cmd_str[CLI_CMD_NAME_MAX - 1] = 0;
  p_cli->cmd_list[index].cmd_func                      = p_func;

  cliToUpper(p_cli->cmd_list[index].cmd_str);

  p_cli->cmd_count++;

  return true;
}

/**
 * @brief 현재 등록된 모든 명령어들을 출력한다.
 */
void cliShowList(cli_args_t *args)
{
  cli_t *p_cli = &cli_node;

  cliPrintf("---------- cmd list ----------\r\n");
  for (int i = 0; i < p_cli->cmd_count; i++)
  {
    cliPrintf("%s\r\n", p_cli->cmd_list[i].cmd_str);
  }
  cliPrintf("------------------------------\r\n");
}
