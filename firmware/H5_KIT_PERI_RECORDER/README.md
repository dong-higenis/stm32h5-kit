# STM32H5 Multi-Protocol Communication Logger

STM32H5 KIT (STM32H563VIT6TR) 기반  
멀티 통신 로그 기록기 펌웨어 프로젝트입니다.

CAN, RS-485, UART 등 다양한 통신 데이터를  
OLED UI를 통해 선택하고,  
선택된 채널의 데이터를 SD 카드에 로그 파일로 저장합니다.

---

## Demo Video

https://youtube.com/shorts/5RydDnx4VE4?si=UWgY_C5Hfatpe-1C

---

## Key Features

### Multi-Protocol Support
- CAN (Implemented)
- RS-485 / RS-232 / UART / LIN (Expandable)

### Real-time Monitoring
- 수신 중인 프레임 OLED 실시간 표시
- SD 카드에 즉시 저장

### FatFs-based SD Logging
- 자동 Mount / Unmount
- 통신별 로그 파일 관리

---

## CAN Logging

- Baudrate: 500 kbps
- 수신 프레임 실시간 표시
- 선택 시 즉시 SD 카드 저장

Log Example


---

## Hardware

| Component | Description |
|----------|-------------|
| MCU      | STM32H563VIT6TR |
| Board    | STM32H5 KIT |
| Display  | OLED |
| Storage  | micro SD Card |
| Network  | Ethernet (lwIP, Planned) |

---

## Planned Features

- lwIP 기반 Ethernet 로그 열람
- 원격 로그 다운로드
- 실시간 네트워크 모니터링

---

## Project Goal

학습용을 넘어,  
참고하시는 분들께 실제로 도움이 되는 프로젝트가 되도록  
지속적인 형상관리와 기능 확장을 목표로 합니다.
