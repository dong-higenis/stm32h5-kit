# STM32H5-KIT Multi-UART & RS485 Communication Example

이 프로젝트는 STM32H5-KIT에서 여러 UART 채널과 RS485 Half-Duplex 통신을 함께 제어하는 확장 예제입니다.

---

## RS485 Half-Duplex 특징

RS485는 산업용 통신 환경에서 널리 사용되는 차동 신호 방식입니다.

| 항목 | 설명 |
|------|------|
| 통신 방식 | Half-Duplex (단방향 전환) |
| 신호선 | A/B 차동 신호 |
| 방향 제어 | DE(Driver Enable) 핀으로 TX/RX 전환 |
| 토폴로지 | 멀티 드롭 버스 (다중 노드 공유) |
| 주의사항 | 동시 송신 시 버스 충돌 발생 가능 |

### 주요 장점
- 장거리 통신 가능 (최대 1200m)
- 노이즈에 강한 차동 신호
- 최대 32개 노드 연결 가능

---

## 개발 환경

- **MCU**: STM32H563-kit(Higenis)
- **IDE**: STM32CubeIDE
- **언어**: C
- **드라이버**: HAL Driver 기반

---

## 사용 방법

STLink UART(USART1) 터미널에서 다음 명령어를 입력하여 각 통신 채널을 테스트할 수 있습니다.

### 명령어 목록

| 명령어 | 동작 설명 |
|--------|----------|
| `LED ON` | 보드 내장 LED 켜기 |
| `LED OFF` | 보드 내장 LED 끄기 |
| `UART6 SEND` | UART6 포트로 `"Hello Uart!"` 메시지 전송 |
| `UART10 SEND` | UART10 포트로 `"Hello Uart!"` 메시지 전송 |
| `RS232` | UART7(RS232 인터페이스)로 `"Hello Uart!"` 전송 |
| `RS485 1` | UART5 → RS485-1 채널로 메시지 송신 |
| `RS485 2` | UART4 → RS485-2 채널로 메시지 송신 |



