# STM32H5-KIT SPI_FLASH 테스트 예제

이 예제는 CLI 기반 시스템에 **SPI FLASH** 읽기/쓰기/삭제 기능을 추가한 예제입니다. 
터미널에서 FLASH 메모리 주소를 지정하여 데이터를 저장하고 다시 읽어 확인할 수 있습니다.

---

## 개발 환경

- **MCU**: STM32H563-kit(Higenis)
- **IDE**: STM32CubeIDE  
- **언어**: C
- **드라이버**: HAL Driver 기반
- **인터페이스** : SPI1
- **FLASH 칩**: Winbond W25Q25(EF 40 19) - 32Mbit SPI NOR FLASH
---

## 주요 특징

- SPI 기반 FLASH 제어(읽기, 쓰기, 섹터 지우기)
- CLI 테스트 인터페이스 제공
- FLASH 상태 레지스터 확인으로 동작 완료 대기
- 페이지 단위 프로그래밍(최대 256 바이트)
- 섹터 단위 지우기(4KB 단위)

---

## FLASH 작업 원리
**읽기 작업**: FLASH의 임의의 주소에서 최대 256바이트까지 데이터를 읽을 수 있습니다. 
CMD_READ(0x03) 명령어를 사용하여 주소와 데이터 길이를 지정한 후 SPI로 데이터를 수신합니다.

**쓰기 작업**: 최대 256바이트(1 페이지)까지 한 번에 쓸 수 있습니다. 
쓰기 작업 전에 CMD_WREN(0x06) 명령어로 Write Enable을 활성화한 후, CMD_PP(0x02) 명령어로 페이지 프로그래밍을 수행합니다. 
쓰기가 완료될 때까지 상태 레지스터의 WIP(Write In Progress) 비트를 폴링하여 대기합니다.

**삭제 작업**: 4KB 섹터 단위로만 지울 수 있습니다. 
CMD_SE(0x20) 명령어를 사용하며, 주소는 반드시 0x1000(4096) 배수여야 합니다. 
삭제 전에 CMD_WREN으로 Write Enable을 활성화해야 하며, 삭제 완료 후 상태 레지스터의 WIP 비트를 폴링하여 동작 완료를 확인합니다.


## 사용 방법

### FLASH 테스트 명령어

터미널(USART1)에서 다음 명령어를 입력하세요:
cli# flash write [addr] [length] [data(0)] [data(1)] ..... [data( length -1 )] 
cli# flash read [addr] [length]
cli# flash erase [addr]


동작 예:
cli# flash write 0x0000 5 0xAA 0xBB 0xCC 0xDD 0xEE
write OK

cli# flash read 0x0000 5
Read from 0x000000 (length: 5)
0x000000: AA BB CC DD EE

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
| **`flash`** |	**`read [addr] [length]`** | **`FLASH에서 데이터 읽기`** |
| **`flash`** |	**`write [addr] [length] [data...]`** | **`FLASH에 데이터 쓰기`** |
| **`flash`** |	**`erase [addr]`**	| **`FLASH 섹터 삭제(4KB 단위)`** |