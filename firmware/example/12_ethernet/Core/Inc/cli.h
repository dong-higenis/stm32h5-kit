#ifndef CLI_H // 중복 인클루드 방지
#define CLI_H

#include <stdbool.h>
#include <stdint.h>

#define CLI_CMD_NAME_MAX 16                                     // 한 명령어의 구성 문자 최대 길이
#define CLI_CMD_LIST_MAX 16                                     // 등록 가능한 명령어 최대 길이
#define CLI_LINE_BUF_MAX 128                                    // 한줄 입력 최대 길이 (엔터를 치기 전까지)
#define CLI_ARGS_MAX     8                                      // 한 명령에서 허용되는 인자 최대 개수 (예 : led on, argc = 2)
#define CLI_LINE_HIS_MAX 8                                      // 이전에 입력한 명령 최대 저장 수

typedef struct cli_args_s
{
  uint16_t argc;                                                // 단어 갯수
  char   **argv;                                                // 단어 목록

  int32_t (*getData)(uint8_t index);                            // 사용자가 입력한 명렁어 추출
  float (*getFloat)(uint8_t index);                             // float 자료형 추출
  char *(*getStr)(uint8_t index);                               // 문자열 추출
  bool (*isStr)(uint8_t index, const char *p_str);              // 같은 문자열인지 확인
} cli_args_t;

bool cliInit(void);                                             // 초기화
bool cliAdd(const char *cmd_str, void (*p_func)(cli_args_t *)); // 커스텀 명령어 추가
void cliProcess(uint8_t rx_data);                               // 내부로직
void cliPrintf(const char *fmt, ...);                           // printf처럼 출력

#endif