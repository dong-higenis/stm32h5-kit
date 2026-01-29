# STM32H5-KIT UART Control Example

이 예제는 USART1(STLink) 인터럽트로 명령을 입력받아, 
명령에 따라 LED 제어, UART6/10 송신, 그리고 RS232로 메시지를 전송하는 UART 제어 확장 예제입니다.

---

## 개발 환경

- **MCU**: STM32H563-kit(Higenis)
- **IDE**: STM32CubeIDE
- **언어**: C
- **드라이버**: HAL Driver 기반

---

##  사용 방법

터미널(STLink, USART1)에서 다음 명령어를 입력하여 각 기능을 테스트할 수 있습니다.

### 명령어 목록

| 입력 명령 | 동작 내용 |
|----------|-----------|
| `LED ON` | 보드 LED 켜기 |
| `LED OFF` | 보드 LED 끄기 |
| `UART6 SEND` | UART6으로 `"Hello Uart!"` 전송 |
| `UART10 SEND` | UART10으로 `"Hello Uart!"` 전송 |
| `RS232` | UART7(RS232)으로 `"Hello Uart!"` 전송 |


