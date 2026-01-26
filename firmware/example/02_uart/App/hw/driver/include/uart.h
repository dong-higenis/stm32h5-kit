#ifndef UART_H_
#define UART_H_

#ifdef __cplusplus
extern "C"
{
#endif


#include "hw_def.h"


#define UART_MAX_CH HW_UART_MAX_CH

  bool     uartInit(void);                                          // UART 초기화
  bool     uartDeInit(void);                                        // UART 종료
  bool     uartIsInit(void);                                        // UART 초기화 여부
  bool     uartOpen(uint8_t ch, uint32_t baud);                     // 지정 채널 UART 열기 및 설정
  bool     uartIsOpen(uint8_t ch);                                  // 해당 UART 채널이 열려있는지 확인
  bool     uartClose(uint8_t ch);                                   // UART 채널 닫기
  uint32_t uartAvailable(uint8_t ch);                               // 수신 링버퍼에 남아있는 데이터 개수 반환
  bool     uartFlush(uint8_t ch);                                   // 수신 버퍼 비우기
  uint8_t  uartRead(uint8_t ch);                                    // 수신 버퍼에서 1바이트 읽기
  uint32_t uartWrite(uint8_t ch, uint8_t *p_data, uint32_t length); // 데이터 송신
  uint32_t uartPrintf(uint8_t ch, const char *fmt, ...);            // printf 형식 문자열 송신
  uint32_t uartGetBaud(uint8_t ch);                                 // 현재 설정된 baudrate 반환
  uint32_t uartGetRxCnt(uint8_t ch);                                // 누적 수신 바이트 수 반환
  uint32_t uartGetTxCnt(uint8_t ch);                                // 누적 송신 바이트 수 반환

#ifdef __cplusplus
}
#endif

#endif
