#include "toy.h"
#include "ui.h"

void toyInit(void)
{
  uiInit();       // UI 상태기계
  sdMountFatFs(); // fatfs 초기화
  canAppInit();   // CAN 초기화
  uartAppInit();  // UART 초기화
  rs485AppInit(); // RS485 초기화
  
}

void toyUpdate(void)
{
  can_msg_t msg;

  for (int i = 0; i < CAN_SEL_MAX_CH; i++)
  {
    if (canMsgAvailable(i)) // can 메시지 수신
    {
      canMsgRead(i, &msg);
      canAppOnRx(i, &msg);
      uiOnButton(msg.id);   // 버튼만 UI에게 전달
    }
  }

  for (int ch = 0; ch < UART_SEL_MAX_CH; ch++)
  {
    uartAppOnRx(ch); // qbuffer → SD 
  }

  for (int ch = 0; ch <RS485_SEL_MAX_CH; ch++)
  {
    rs485AppOnRx(ch);
  }
  uiDraw();          // oled 출력
}
