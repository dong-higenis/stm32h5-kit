# STM32H5-KIT LwIP Ethernet 통신 예제

이 예제는 CLI 기반 시스템에 **LwIP (Lightweight IP) 프로토콜 스택**을 추가하여 **Ethernet 통신** 기능을 구현한 예제입니다. 
터미널에서 네트워크 상태를 확인하고 DHCP를 통한 자동 IP 할당 또는 정적 IP 설정을 테스트할 수 있습니다.

---

## 개발 환경

- **MCU**: STM32H563-kit(Higenis)
- **IDE**: STM32CubeIDE  
- **언어**: C
- **드라이버**: HAL Driver 기반
- **미들웨어**: LwIP 2.1.x (NO_SYS 모드)
- **PHY 칩**: LAN8742A (RMII 인터페이스)
- **인터페이스**: ETH (Ethernet MAC)
- **통신 속도**: 10/100 Mbps (Auto-negotiation)
---

## 주요 특징

- LwIP 프로토콜 스택: TCP/IP 네트워크 통신 지원
- DHCP 자동 IP 할당: 공유기/라우터로부터 자동으로 IP 주소 획득
- 정적 IP Fallback: DHCP 타임아웃 시 자동으로 정적 IP로 전환
- 실시간 링크 감지: 이더넷 케이블 연결/해제 자동 감지
- 주기적 상태 모니터링: 5초마다 네트워크 상태 자동 출력

---

## Ethernet 작동 원리

**Ethernet (이더넷)** 은 가장 널리 사용되는 유선 LAN 통신 프로토콜로, OSI 7계층 중 물리 계층과 데이터 링크 계층을 담당합니다.

**LwIP (Lightweight IP)**: 임베디드 시스템을 위한 경량 TCP/IP 프로토콜 스택으로, 메모리 사용량이 적고 속도가 빠릅니다.

**RMII (Reduced Media Independent Interface)**: STM32와 PHY 칩(LAN8742A) 사이의 인터페이스로, 7개의 핀으로 통신합니다.

## 프레임 구조

모든 Ethernet 프레임은 다음 순서로 구성됩니다:

|**`[Preamble]`**| `[SFD]` | `[Destination MAC]` | `[Source MAC]` | `[EtherType]` | `[Payload]` | `[FCS]` |


### DHCP (Dynamic Host Configuration Protocol)

DHCP는 네트워크 장치에 IP 주소를 자동으로 할당하는 프로토콜입니다.

DHCP 상태 머신:

**DHCP_START**: DHCP 프로세스 시작

**DHCP_WAIT_ADDRESS**: DHCP 서버 응답 대기 (최대 4회 재시도)

**DHCP_ADDRESS_ASSIGNED**: IP 주소 할당 성공

**DHCP_TIMEOUT**: 타임아웃 발생 → 정적 IP로 전환

**DHCP_LINK_DOWN**: 이더넷 케이블 연결 끊김

네트워크 주소 구성

IP Address: 네트워크에서 장치를 식별하는 주소 (예: 192.168.1.100)

Netmask: 네트워크와 호스트 부분을 구분 (예: 255.255.255.0)

Gateway: 외부 네트워크로 나가는 관문 (예: 192.168.1.1)

MAC Address: 하드웨어 고유 주소 (예: 00:80:E1:00:00:00)


## 포팅 가이드

STM32H5 LwIP 포팅 가이드
https://community.st.com/t5/stm32-mcus/how-to-use-the-lwip-ethernet-middleware-on-the-stm32h5-series/ta-p/691100

## 프로젝트 구조
project/
├── lwip/
│   ├── src/           # LwIP 코어 소스
│   ├── app/           # 사용자 애플리케이션 계층
│   │   ├── lwip.c     # LwIP 초기화 및 DHCP 관리
│   │   └── lwip.h
│   └── target/        # 하드웨어 추상화 계층
│       ├── ethernetif.c  # Ethernet 드라이버 인터페이스
│       └── ethernetif.h
├── Drivers/
│   └── BSP/
│       └── lan8742/   # LAN8742 PHY 드라이버
└── Core/
    └── Src/
        └── main.c     # 메인 애플리케이션


## 핵심 파일 수정 사항
A. ethernetif.c

STM32H5용 인터럽트 모드 사용 (HAL_ETH_Start_IT / HAL_ETH_Stop_IT)
ETH 핸들 이름 통일 (heth)
GPIO 핀 설정 (보드에 맞게 TX_EN, TXD0, TXD1 핀 수정)
MAC 주소 설정 (기본값: 00:80:E1:00:00:00)

B. lwip.c

DHCP 상태 머신 구현
정적 IP Fallback 기능 추가
링크 상태 콜백 처리
주기적인 타이머 처리 (ARP, DHCP, TCP 재전송 등)

CubeMX 설정

ETH 활성화: RMII 모드
GPIO 설정: RMII 핀 (MDC, MDIO, REF_CLK, CRS_DV, RXD0/1, TX_EN, TXD0/1)
인터럽트 우선순위: ETH_IRQn (우선순위 5~7)
빌드 주의사항

LAN8742 드라이버 포함
