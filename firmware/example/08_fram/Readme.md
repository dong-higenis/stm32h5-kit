# STM32H5-KIT FDCAN 통신 테스트 예제

이 예제는 CLI 기반 시스템에 I2C **FRAM 읽기/쓰기** 테스트 기능을 추가한 예제입니다.
터미널에서 FRAM 메모리 주소를 지정하여 데이터를 저장하고 다시 읽어 확인할 수 있습니다.

---

## 개발 환경

- **MCU**: STM32H563-kit(Higenis)
- **IDE**: STM32CubeIDE  
- **언어**: C
- **드라이버**: HAL Driver 기반
- **인터페이스** : I2C4
---

## 주요 특징

- I2C 기반 FRAM 제어
- CLI 테스트 인터페이스 제공
- 1바이트 단위 Read / Write 지원
- ACK 기반 디바이스 연결 확인
- FRAM 특성 반영 (Write Delay 없음)

---

## 사용 방법

### CAN 테스트 명령어

터미널(USART1)에서 다음 명령어를 입력하세요:
cli# fram info
cli# fram write [addr] [data]
cli# fram read [addr]


동작 예:
cli# fram write 16 55
write OK

cli# fram read 16
addr 0x0010 -> 0x37

### 전체 명령어 목록

| 명령어 | 인자 | 동작 설명 |
|--------|------|-----------|
| `help` | - | 등록된 모든 명령어 목록 출력 |
| `led` | `on` / `off` / `toggle` | LED 제어 |
| `uart6` | - | UART6으로 메시지 전송 |
| `uart10` | - | UART10으로 메시지 전송 |
| `rs232` | - | UART7(RS232)로 메시지 전송 |
| `rs485` | `1` / `2` | RS485-1 또는 RS485-2로 메시지 전송 |
| `oled` | `test` | OLED 실시간 입력 모드 |
| `can` | `test 1~2` | CAN1/CAN2 루프백 테스트 |
| **`fram`** | **`info`** | **`FRAM INIT 여부 확인`** |
| **`fram`** | **`read [addr]`** | **` 지정된 주소에서 1바이트 데이터 읽기`** |
| **`fram`** | **`write [addr] [data]`** | **`지정된 주소에 1바이트 데이터 쓰기`** |   



