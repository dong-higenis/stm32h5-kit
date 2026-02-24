# STM32H5-KIT SD 카드 FatFS 예제

이 예제는 CLI 기반 시스템에 **SD 카드 파일 시스템(FatFS)** 읽기/쓰기 기능을 추가한 예제입니다. 
터미널에서 SD 카드에 파일을 생성하고, 데이터를 저장하며, 저장된 내용을 다시 읽어 확인할 수 있습니다.

---

## 개발 환경

- **MCU**: STM32H563-kit(Higenis)
- **IDE**: STM32CubeIDE  
- **언어**: C
- **드라이버**: HAL Driver 기반
- **인터페이스**: SDMMC1 (4-bit Wide Bus)
- **파일 시스템**: FatFS R0.14 이상
- **SD 카드**: FAT32 포맷 권장
---

## 주요 특징

- SDMMC 인터페이스를 통한 고속 SD 카드 액세스
- FatFS 파일 시스템 지원 (파일 생성, 읽기, 쓰기, 삭제)
- 자동 마운트 기능 (명령 실행 시 자동으로 마운트 시도)

---

## FLASH 작업 원리
**초기화 작업**: SDMMC1 주변장치를 초기화하고 SD 카드의 물리적 상태를 확인합니다. 
HAL_SD_Init() 함수를 사용하여 SD 카드와 통신을 설정하며, 카드 타입, 용량, 블록 크기 등의 정보를 읽어옵니다.

**마운트 작업**: FatFS의 f_mount() 함수를 사용하여 SD 카드의 파일 시스템을 시스템에 연결합니다. 
마운트가 완료되면 파일 및 디렉토리에 접근할 수 있으며, 드라이브 번호 "0:"을 통해 루트 디렉토리를 참조합니다.

**파일 쓰기**: FatFS의 f_open() 함수로 파일을 열고, f_write() 함수로 데이터를 씁니다. 
FA_OPEN_APPEND 플래그를 사용하면 파일이 없으면 새로 생성하고, 있으면 파일 끝에 데이터를 추가합니다. 
쓰기 작업 후 f_sync()를 호출하여 RAM 버퍼의 데이터를 실제 SD 카드에 기록하므로, 전원이 갑자기 꺼져도 데이터 손실을 최소화할 수 있습니다.

**파일 읽기**: f_open() 함수로 파일을 FA_READ 모드로 열고, f_read() 함수로 데이터를 읽습니다. 
파일 크기가 큰 경우 128바이트씩 반복해서 읽어 메모리 사용량을 최소화하며, 읽은 데이터는 즉시 터미널에 출력됩니다.

**언마운트 작업**: 작업 완료 후 f_mount(NULL, "0:", 1)을 호출하여 파일 시스템을 안전하게 언마운트합니다. 
이를 통해 파일 시스템 구조의 무결성을 보장하고 SD 카드를 안전하게 제거할 수 있습니다.


## 사용 방법

### SD 카드 테스트 명령어

터미널(USART1)에서 다음 명령어를 입력하세요:
cli# sd info
cli# sd mount
cli# sd unmount
cli# sd write [filename] [data]
cli# sd read [filename]


동작 예:
# SD 카드 정보 확인
cli# sd info

=== SD Card Info ===

SD Init: OK

FatFs Mounted: YES

Card Type: 1

Card Size: 7680 MB (7.5 GB)

Block Count: 15728640

Block Size: 512 bytes

# 파일 시스템 마운트
cli# sd mount

FatFs Mount OK

# 파일에 데이터 쓰기 (자동으로 줄바꿈 추가)
cli# sd write log.txt "System started"

Write OK: log.txt

cli# sd write log.txt "Temperature: 25C"

Write OK: log.txt

cli# sd write log.txt "Sensor OK"

Write OK: log.txt

# 파일 내용 읽기
cli# sd read log.txt

--- File Content: log.txt ---

System started

Temperature: 25C

Sensor OK

--- End ---

# 다른 파일 생성
cli# sd write data.csv "time,temp,humidity"

Write OK: data.csv

cli# sd write data.csv "10:00,25,60"

Write OK: data.csv

# 파일 시스템 언마운트
cli# sd unmount

FatFs Unmount OK

# diskio.c 구현 요약

- FatFS와 SD 카드 하드웨어 사이의 저수준 인터페이스를 구현합니다:
- disk_initialize(): SD 카드 초기화 상태를 확인하고 FatFS에 전달합니다.
- disk_read(): 지정된 섹터에서 데이터를 읽어 버퍼에 저장합니다. HAL_SD_ReadBlocks() 함수를 사용하며, DMA 또는 폴링 방식을 선택할 수 있습니다.
- disk_write(): 버퍼의 데이터를 지정된 섹터에 씁니다. HAL_SD_WriteBlocks() 함수를 사용하며, 쓰기 완료 후 카드 상태를 확인합니다.
- disk_ioctl(): 섹터 개수, 섹터 크기, 동기화 등의 제어 명령을 처리합니다. FatFS가 파일 시스템을 초기화하고 최적화하는 데 필요한 정보를 제공합니다.
- get_fattime(): 파일의 타임스탬프를 설정하기 위한 현재 시간을 반환합니다. RTC가 있으면 실제 시간을, 없으면 고정된 시간을 반환합니다.

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
| **`sd`** |  **`info`** |	`SD 카드 정보 출력 (용량, 블록 수, 마운트 상태)` |
| **`sd`** |  **`mount`** |	`FatFS 파일 시스템 마운트` |
| **`sd`** |  **`unmount`** |	`FatFS 파일 시스템 언마운트` |
| **`sd`** |  **`write [file] [data]`** |	`SD 카드 파일에 데이터 쓰기 (자동 줄바꿈)` |
| **`sd`** |  **`read [file]`** |	`SD 카드 파일 읽기 및 출력` |