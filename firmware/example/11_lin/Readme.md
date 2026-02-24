# STM32H5-KIT LIN 통신 예제

이 예제는 CLI 기반 시스템에 **LIN (Local Interconnect Network)** 통신 기능을 추가한 예제입니다. 
터미널에서 LIN 프레임을 송신하고 수신하여 차량용 저속 통신 프로토콜을 테스트할 수 있습니다.

---

## 개발 환경

- **MCU**: STM32H563-kit(Higenis)
- **IDE**: STM32CubeIDE  
- **언어**: C
- **드라이버**: HAL Driver 기반
- **인터페이스**: UART2, UART3, UART4 (LIN 모드)
- **통신 속도**: 9600 bps (LIN 표준)
- **물리 스위치**: LIN3 채널은 물리 스위치로 RS485/LIN 선택 가능
---

## 주요 특징

- 3채널 LIN 통신 지원 (LIN1: UART2, LIN2: UART3, LIN3: UART4)
- LIN 프레임 구조 완벽 구현 (Break + Sync + PID + Data + Checksum)
- 자동 체크섬 계산 (Classic LIN Checksum)
- 실시간 수신 모니터링 (Break, Sync, PID, Data, Checksum 검증)

---

## LIN 작업 원리

**LIN (Local Interconnect Network)** 은 자동차 내부의 저속 통신에 사용되는 단일 마스터, 
다중 슬레이브 방식의 직렬 통신 프로토콜입니다. 
CAN 버스보다 저렴하고 간단하여 도어, 윈도우, 시트 제어 등에 널리 사용됩니다.

**프레임 구조**: 모든 LIN 프레임은 [Break] [Sync] [PID] [Data 0~8 bytes] [Checksum] 순서로 구성됩니다.

**Break 신호**: 프레임의 시작을 알리는 신호로, 최소 13비트 이상 연속 LOW 상태를 유지합니다. 
HAL_LIN_SendBreak() 함수를 사용하여 자동으로 생성하며, 버스 상의 모든 슬레이브 노드에게 새로운 프레임이 시작됨을 알립니다.

**Sync Byte (0x55)**: 동기화 바이트로, 모든 LIN 프레임에서 고정값 0x55 (01010101b)를 전송합니다. 
슬레이브는 이 바이트의 타이밍을 측정하여 마스터의 전송 속도에 자동으로 동기화합니다.

**PID (Protected Identifier)**: 프레임의 ID(0~63)에 패리티 비트 2개를 추가한 8비트 값입니다. 
어떤 데이터인지 식별하고 통신 오류를 검출하는 데 사용됩니다.

**Data (0~8 bytes)**: 실제 전송할 데이터로, 0바이트부터 최대 8바이트까지 가능합니다. 
센서 값, 제어 명령, 상태 정보 등이 포함됩니다.

**Checksum**: 프레임의 무결성을 검증하기 위한 체크섬입니다. 
Classic LIN Checksum 방식을 사용하며, PID와 데이터 바이트의 합에 캐리를 처리한 후 1의 보수를 계산합니다.

## 사용 방법

### LIN 통신 테스트 명령어

터미널(USART1)에서 다음 명령어를 입력하세요:
cli# lin info
cli# lin tx [ch] [pid] [data0] [data1] ...
cli# lin rx [ch]


동작 예:

# LIN 프레임 전송 (채널 0, PID 0x34, 데이터 3바이트)
cli# lin tx 0 0x34 0x11 0x22 0x33

LIN TX ch0: PID=0x34, Len=3
  Data: 0x11 0x22 0x33 
  Checksum: 0x65
  -> Sent!

# LIN 수신 모니터링 (채널 0)
cli# lin rx 0

=== LIN RX Monitor (ch0) ===
Press 'q' to quit
Waiting for data...

[RX] [BREAK]

[RX] [SYNC]

[RX] [PID:0x34]

[RX] 0x11

[RX] 0x22

[RX] 0x33

[RX] [Checksum Received = 0x65]

[RX] Checksum OK 

# 센서 값 전송 예시 (채널 1, PID 0xD6, 데이터 4바이트)
cli# lin tx 1 0xD6 0x01 0x02 0x03 0x04

LIN TX ch1: PID=0xD6, Len=4
  Data: 0x01 0x02 0x03 0x04 
  Checksum: 0x28
  -> Sent!

# 헤더만 전송 (데이터 없음)
cli# lin tx 2 0x3C

LIN TX ch2: PID=0x3C, Len=0
  Data: 
  Checksum: 0xC3
  -> Sent!

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
| `fram` | `info` | FRAM INIT 여부 확인 |
| `flash` |	`read [addr] [length]` | FLASH에서 데이터 읽기 |
| `flash` |	`write [addr] [length] [data...]` | FLASH에 데이터 쓰기 |
| `flash` |	`erase [addr]`	|  FLASH 섹터 삭제(4KB 단위) |
| `sd` |  `info` |	SD 카드 정보 출력 (용량, 블록 수, 마운트 상태) |
| `sd` |  `mount` | FatFS 파일 시스템 마운트 |
| `sd` |  `unmount` |	FatFS 파일 시스템 언마운트 |
| `sd` |  `write [file] [data]` |	SD 카드 파일에 데이터 쓰기 (자동 줄바꿈) |
| `sd` |  `read [file]` |  SD 카드 파일 읽기 및 출력 |
| **`lin`** |  **`info`** | `LIN 사용 가능 여부 및 사용 가능 채널` |
| **`lin`** | **`tx [ch] [pid] [data0] ...`** |	`LIN 프레임 전송 (Break + Sync + PID + Data + Checksum)` |
| **`lin`** |  **`rx [ch]`** | `LIN 수신 모니터 (실시간 프레임 파싱 및 체크섬 검증)` |