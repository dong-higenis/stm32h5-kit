# STM32H5 KIT (STM32H563VIT6TR)

STM32H563VIT6TR MCU를 중심으로 CAN / LIN / RS485 / RS232 / Ethernet / USB / SDMMC 등을 지원하는
멀티 인터페이스 개발보드입니다. 산업용·차량용 통신 테스트 및 게이트웨이 개발을 목적으로 설계되었습니다.

---

## 1. MCU

- **MCU**: STM32H563VIT6TR (Arm Cortex-M33)
- **패키지**: LQFP-100
- **외부 크리스탈**
  - Main Clock: 25 MHz
  - RTC Clock: 32.768 kHz
- **BOOT 구성**
  - BOOT0 스위치 및 풀다운 저항
- **디버그**
  - SWD (SWDIO / SWCLK)
  - NRST 버튼

---

## 2. 전원 구성

- **입력 전원**
  - DC Jack
  - USB Type-C (VBUS)
- **전원 변환**
  - Buck Converter: SY8205FC (+5V)
  - LDO: AMS1117-3.3 (+3.3V)
- **전원 분리**
  - +3.3V (Digital)
  - +3.3VA (Analog)
- **RTC VBAT**
  - 외부 배터리 입력 지원

---

## 3. 통신 인터페이스

### Ethernet (RMII)
- PHY: LAN8742AI
- 인터페이스: RMII
- RJ45 MagJack (Link / Activity LED)
- ESD 보호 및 임피던스 매칭 회로 적용

### CAN
- 2채널 (CAN1 / CAN2)
- CAN Transceiver: TJA1051
- 120Ω 종단저항 DIP 스위치 선택
- ESD 보호 및 공통모드 초크 적용

### LIN
- 3채널 (LIN1 / LIN2 / LIN3)
- LIN Transceiver: TJA1028
- Main/Sub 모드 선택 가능

### RS485
- 2채널
- Transceiver: SP3485
- 120Ω 종단저항 선택 가능

### RS232
- Transceiver: MAX3232 계열
- DB9 Female 커넥터

---

## 4. USB

- USB Type-C
- USB FS Device
- ESD 보호 회로 구성
- CC 저항으로 기본 Device 모드 설정

---

## 5. 저장 장치

### microSD
- SDMMC 인터페이스
- Card Detect 지원

### SPI Flash
- W25Q256 (256Mbit)
- 데이터 로깅 / 외부 저장용

### FRAM
- 2KB I2C FRAM
- 설정 데이터 보존용

---

## 6. 사용자 인터페이스

- OLED (1.3", I2C)
- User LED
- Buzzer (PWM)
- Reset / Boot 버튼
- 통신 설정 DIP 스위치

---

## 7. 확장

- GPIO 확장 헤더
- UART / I2C / SPI 핀 헤더
- UART4 LIN/RS485 선택 헤더

---

## 8. 활용 예

- 차량용 CAN/LIN 통신 테스트
- 산업용 통신 게이트웨이
- Ethernet ↔ Fieldbus 브리지
- 데이터 로거 및 테스트 장비

---

## 9. 참고

- 본 프로젝트는 하드웨어 중심 개발보드입니다.
- 회로도 기반으로 작성된 문서입니다.
