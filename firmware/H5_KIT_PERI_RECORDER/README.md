STM32H5 KIT (STM32H563VIT6TR) 기반의
멀티 통신 로그 기록기 펌웨어 프로젝트입니다.

CAN, RS-485, UART 등 다양한 통신 데이터를
OLED UI를 통해 선택하고,
선택된 채널의 데이터를 SD 카드에 로그 파일로 저장합니다.

Demo Video

https://youtube.com/shorts/5RydDnx4VE4?si=QMVPogZyqkCvZc-Y

Key Features

Multi-Protocol Support

CAN (Implemented)

RS-485 / RS-232 / UART / LIN (Expandable)

Real-time Monitoring

수신 중인 프레임을 OLED에 실시간 표시

버튼 입력으로 현재 프레임을 SD 카드에 즉시 저장

FatFs 기반 SD Logging

자동 Mount / Unmount

통신별 로그 파일 관리

CAN Logging

Baudrate: 500 kbps

수신 프레임 실시간 표시

선택 시 즉시 SD 카드 저장

Example:

12345678  ID:1A3  LEN:8  11-22-33-44-55-66-77-88

Hardware

MCU: STM32H563VIT6TR

Board: STM32H5 KIT

Display: OLED

Storage: micro SD Card

Network (Planned): Ethernet (lwIP)

Planned Features

lwIP 기반 Ethernet 로그 열람

원격 로그 다운로드

실시간 네트워크 모니터링

Project Goal

참고하시는 분들께 도움이 되기 위한,
펌웨어로써 계속 형상관리를 할 수 있게 목표를 잡고 있습니다.