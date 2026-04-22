# H5 CAN Logger

## Overview

해당 펌웨어는 현재 CAN만 로깅하게 구현되어 있습니다.
각 하드웨어 모듈은 개발의 진행도에 따라 특수 목적으로도 쓰일수있게 구현된 채로 남겼습니다.

- `CAN1` -> `log/CAN1.csv`
- `CAN2` -> `log/CAN2.csv`

## How To Use

1. SD카드를 삽입합니다.
2. STM32H5-KIT보드에 전원을 공급합니다. (USB-C 5V) 및 (STLINK)
3. CLI UART를 받아보기 위해 Teraterm등에서 Baudrate 115200으로 설정하여 open합니다.
4. 원하는 CAN 채널에 대해 CAN_H, CAN_L를 연결해주어 메시지를 받게 만들어 줍니다.
5. 첫 메시지가 SD카드에 기록되면 아래와 같은 메시지가 출력됩니다.

```text
[LOG] CAN logging started: log/CAN1.csv
```

만약 `CAN2` 를 수신 받았다면, SD 카드 내 `log/CAN2.csv`에 저장됩니다.

## SD Card Layout

해당 펌웨어는 SD카드의 루트 디렉토리에 아래와 같이 파일을 생성합니다.

- `log/CAN1.csv`
- `log/CAN2.csv`

각 채널마다 독립적인 로그 파일을 다루게 됩니다.

## CSV Format

모든 파일은 `.csv` 포맷으로 저장되며 첫 행에는 아래와 같은 헤더를 가지고 있습니다.

```text
timestamp,dir,id,dlc,d0,d1 ... d63,err
```

Field meanings:

- `timestamp`: `millis()` 수신 시각
- `dir`: 현재는 항상 `RX`
- `id`: CAN identifier
- `dlc`: data 길이
- `d0`..`d63`: data 
- `err`: CAN error 스냅샷

Example row:

```text
123456,RX,0x00000123,8,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88, ... ,0
```

만약 64바이트보다 작은 데이터를 수신받은 경우 해당 데이터 행을 비워진 상태로 기록됩니다.

## Start Message Rule

"CAN Logging started"메시지는 아래 조건이 모두 만족된 뒤에만 출력됩니다.

1. CAN 프레임이 실제로 수신되었고,
2. 해당 프레임이 큐에서 꺼내져 CSV 형식으로 변환되었으며,
3. SD 카드 쓰기가 실제로 성공했을 때

