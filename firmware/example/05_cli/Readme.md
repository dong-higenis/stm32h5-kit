# STM32H5-KIT CLI (Command Line Interface) Example

이 예제는 이전 예제들을 종합하여 개발에 편리한 CLI(Command Line Interface)를 구현합니다. 
터미널에서 명령어를 입력하면 LED 제어, 다중 UART 송신, RS232/RS485 통신을 실행할 수 있습니다.
해당 CLI 로직은 유튜버 Baram님의 코드를 참조했습니다.
---

## 주요 특징

- **명령어 기반 제어**: 문자열 파싱을 통한 직관적인 인터페이스
- **명령어 히스토리**: ↑/↓ 키로 이전 명령어 탐색 (최대 16개)
- **커서 이동 및 편집**: ←/→ 키로 커서 이동, Backspace/Delete 지원
- **확장 가능한 구조**: 새 명령어를 쉽게 추가할 수 있는 구조
- **ANSI 이스케이프 시퀀스 지원**: 터미널 제어 문자 처리

---


## 개발 환경

- **MCU**: STM32H563-kit(Higenis)
- **IDE**: STM32CubeIDE
- **언어**: C
- **드라이버**: HAL Driver 기반

---

## 사용 방법

터미널(USART1)에서 명령어를 입력하세요.

### 기본 명령어

| 명령어 | 인자 | 동작 설명 |
|--------|------|-----------|
| `help` | - | 등록된 모든 명령어 목록 출력 |
| `led` | `on` / `off` / `toggle` | LED 켜기 / 끄기 / 500ms 주기 토글 |
| `uart6` | - | UART6으로 `"[SENT]Hello Uart!"` 전송 |
| `uart10` | - | UART10으로 `"[SENT]Hello Uart!"` 전송 |
| `rs232` | - | UART7(RS232)로 `"[RS232]Hello Uart!"` 전송 |
| `rs485` | `1` / `2` | RS485-1 또는 RS485-2로 메시지 전송 |

