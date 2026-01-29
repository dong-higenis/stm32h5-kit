# STM32H5-KIT Multi-UART Interrupt Control Example

이 예제는 USART1 인터럽트로 명령을 입력받고, 명령에 따라 LED를 제어하거나 USART6 / USART10으로 메시지를 전송하는 멀티 UART 인터럽트 기반 제어 예제입니다.

---

## 개발 환경

- **MCU**: STM32H563-kit(Higenis)
- **IDE**: STM32CubeIDE
- **언어**: C
- **드라이버**: HAL Driver 기반

---

## 사용 방법

터미널(USART1)에서 다음 명령어를 입력하여 각 기능을 테스트할 수 있습니다.

### 명령어 목록

| 입력 명령 | 동작 내용 |
|----------|-----------|
| `LED ON` | 보드 LED 켜기 |
| `LED OFF` | 보드 LED 끄기 |
| `UART6 SEND` | USART6으로 메시지 전송 |
| `UART10 SEND` | USART10으로 메시지 전송 |


