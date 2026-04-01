
#include "lwip.h"
#include "lwip/init.h"
#include "lwip/netif.h"
#if defined ( __CC_ARM )  /* MDK ARM Compiler */
#include "lwip/sio.h"
#endif /* MDK ARM Compiler */
#include "ethernetif.h"

#include "lwip/timeouts.h"
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/tcp.h"
#include "lwip/dhcp.h"
#include <string.h>
#include "main.h"

#include "cli.h" // @@cli 사용

struct netif gnetif; // 전역 네트워크 인터페이스

uint32_t EthernetLinkTimer;

static bool is_link = false;  // @@링크 상태 플래그

#if LWIP_DHCP
#define DHCP_OFF                   (uint8_t) 0
#define DHCP_START                 (uint8_t) 1
#define DHCP_WAIT_ADDRESS          (uint8_t) 2
#define DHCP_ADDRESS_ASSIGNED      (uint8_t) 3
#define DHCP_TIMEOUT               (uint8_t) 4
#define DHCP_LINK_DOWN             (uint8_t) 5
#define MAX_DHCP_TRIES  4

uint32_t DHCPfineTimer = 0;
uint8_t DHCP_state = DHCP_OFF;
#endif

// @@ 아래 매크로 추가 (동적 ip 할당 실패시 받을 정적 ip 매크로)
#ifndef IP_ADDR0
#define IP_ADDR0    192
#define IP_ADDR1    168
#define IP_ADDR2    1
#define IP_ADDR3    100
#endif

#ifndef NETMASK_ADDR0
#define NETMASK_ADDR0   255
#define NETMASK_ADDR1   255
#define NETMASK_ADDR2   255
#define NETMASK_ADDR3   0
#endif

#ifndef GW_ADDR0
#define GW_ADDR0   192
#define GW_ADDR1   168
#define GW_ADDR2   1
#define GW_ADDR3   1
#endif

/**
  * @@
  * @brief  링크 상태 확인 함수
  * @retval bool: true = 링크 연결, false = 링크 끊김
  */
bool lwipIsLink(void)
{
  return is_link;
}

/**
 * @brief LwIP 네트워크 인터페이스에서 현재 IP 주소를 가져옵니다
 * 
 * @param ip_buffer IP 주소를 저장할 문자열 버퍼 (최소 16바이트 필요)
 * @param size      버퍼 크기
 * @return true     IP 주소 할당 완료 및 네트워크 인터페이스 활성화 상태
 * @return false    IP 주소 미할당 (DHCP 대기 중) 또는 네트워크 다운 상태
 * 
 * @note IP 주소가 없으면 "0.0.0.0"을 반환하고 false 리턴
 * @note 버퍼 크기는 최소 16바이트 필요 (예: "255.255.255.255\0")
 */
bool lwipGetIPAddress(char *ip_buffer, size_t size)
{
  // - ip_buffer가 NULL 포인터인지 확인
  // - size가 최소 16바이트(IPv4 문자열 최대 길이 "255.255.255.255" + '\0') 이상인지 확인
  if (ip_buffer == NULL || size < 16) 
  {
    return false;
  }

  // 네트워크 인터페이스 상태 확인 및 IP 주소 가져오기
  if (netif_is_up(&gnetif) && !ip4_addr_isany_val(*netif_ip4_addr(&gnetif))) // 네트워크 인터페이스가 활성화(UP) 상태인지 확인 && IP 주소가 0.0.0.0이 아닌지 확인
  {
    // IP 주소를 문자열로 변환 -> ip_buffer에
    ip4addr_ntoa_r(netif_ip4_addr(&gnetif), ip_buffer, size);
    return true;
  }
  
  // 네트워크가 비활성화 상태이거나 IP 주소가 할당되지 않은 경우
  strncpy(ip_buffer, "0.0.0.0", size);
  return false;
}

/**
  * @brief  DHCP 상태 문자열 반환
  * @retval const char*: DHCP 상태 문자열
  */
const char* lwipGetDHCPStateString(void)
{
  switch(DHCP_state)
  {
    case DHCP_OFF:              return "OFF";
    case DHCP_START:            return "START";
    case DHCP_WAIT_ADDRESS:     return "WAIT_ADDRESS";
    case DHCP_ADDRESS_ASSIGNED: return "ASSIGNED";
    case DHCP_TIMEOUT:          return "TIMEOUT";
    case DHCP_LINK_DOWN:        return "LINK_DOWN";
    default:                    return "UNKNOWN";
  }
}

/**
 * @brief  이더넷 링크 상태 변경 시 호출되는 콜백 함수
 * 
 * @param  netif 네트워크 인터페이스 구조체 포인터
 * 
 * @note 이 함수는 LwIP 스택에서 자동으로 호출됩니다
 * @note 케이블 연결/해제 시 즉시 반응하여 DHCP 상태를 업데이트합니다
 * 
 * @details
 * 동작 흐름:
 * 1. 링크 UP (케이블 연결됨)
 *    → is_link = true 설정
 *    → DHCP 프로세스 시작 (DHCP_START)
 *    → 로그 출력: "Ethernet link UP"
 * 
 * 2. 링크 DOWN (케이블 해제됨)
 *    → is_link = false 설정
 *    → DHCP 중단 (DHCP_LINK_DOWN)
 *    → 로그 출력: "Ethernet link DOWN"
 */
void ethernet_link_status_updated(struct netif *netif)
{
  if (netif_is_link_up(netif))
  {
    is_link = true;
    DHCP_state = DHCP_START;
    cliPrintf("[  ] Ethernet link UP\n");
  }
  else
  {
    is_link = false;
    DHCP_state = DHCP_LINK_DOWN;
    cliPrintf("[  ] Ethernet link DOWN\n");
  }
}

#if LWIP_NETIF_LINK_CALLBACK
/**
  * @brief  Ethernet 링크 주기적 체크
  * @param  netif: 네트워크 인터페이스
  * @retval None
  */
void Ethernet_Link_Periodic_Handle(struct netif *netif)
{
  /* Ethernet Link every 100ms */
  if (HAL_GetTick() - EthernetLinkTimer >= 100)
  {
    EthernetLinkTimer = HAL_GetTick();
    ethernet_link_check_state(netif);
  }
}
#endif

#if LWIP_DHCP
/**
 * @brief  DHCP 상태 머신 처리 함수
 * 
 * @param  netif 네트워크 인터페이스 구조체 포인터
 * 
 * @details
 * DHCP 상태에 따라 다음 동작을 수행합니다:
 * - DHCP_START: DHCP 프로세스 시작 (IP 주소 초기화 및 DHCP 요청)
 * - DHCP_WAIT_ADDRESS: DHCP 서버 응답 대기 (할당 성공 또는 타임아웃 처리)
 * - DHCP_LINK_DOWN: 링크 다운 시 DHCP 중지
 * 
 * @note 이 함수는 DHCP_Periodic_Handle()에서 500ms마다 호출됩니다
 * @note DHCP 타임아웃 시 자동으로 정적 IP로 전환합니다
 */
void DHCP_Process(struct netif *netif)
{
  ip_addr_t ipaddr;   // IP 주소 구조체
  ip_addr_t netmask;  // 서브넷 마스크 구조체
  ip_addr_t gw;       // 게이트웨이 주소 구조체
  struct dhcp *dhcp;  // DHCP 제어 구조체 포인터
  char ip_str[16];    // IP 주소 문자열 버퍼 (예: "192.168.1.100")

  switch (DHCP_state)
  {
    case DHCP_START:
    {
      cliPrintf("[  ] Looking for DHCP server...\n");
      
      ip_addr_set_zero_ip4(&netif->ip_addr);   // IP 주소를 0.0.0.0으로 초기화
      ip_addr_set_zero_ip4(&netif->netmask);   // 서브넷 마스크를 0.0.0.0으로 초기화
      ip_addr_set_zero_ip4(&netif->gw);        // 게이트웨이를 0.0.0.0으로 초기화
      
      // DHCP 클라이언트 시작
      // 내부적으로 DHCP DISCOVER 메시지를 브로드캐스트하여 DHCP 서버 탐색 시작
      dhcp_start(netif);
      DHCP_state = DHCP_WAIT_ADDRESS;
    }
    break;

    case DHCP_WAIT_ADDRESS:
    {
      // DHCP 서버로부터 IP 주소가 할당되었는지 확인
      // dhcp_supplied_address(): DHCP로 IP를 받았으면 true 반환
      if (dhcp_supplied_address(netif))
      {
        DHCP_state = DHCP_ADDRESS_ASSIGNED;
        
        // 할당된 IP 주소를 문자열로 변환하여 출력
        ip4_addr_set_u32(&ipaddr, netif_ip4_addr(netif)->addr);
        ip4addr_ntoa_r(&ipaddr, ip_str, sizeof(ip_str));
        cliPrintf("[OK] DHCP assigned: %s\n", ip_str);
      }
      else // 실패시 정적 ip로 변환
      {
        // 네트워크 인터페이스에서 DHCP 제어 구조체 가져오기
        // 이 구조체에는 DHCP 상태, 재시도 횟수, 타이머 정보 등이 저장됨
        dhcp = (struct dhcp *)netif_get_client_data(netif, LWIP_NETIF_CLIENT_DATA_INDEX_DHCP);

        // 4번의 재시도 후 타임아웃
        if (dhcp->tries > MAX_DHCP_TRIES)
        {
          DHCP_state = DHCP_TIMEOUT;

          // 정적 IP 주소 설정 (사전 정의된 매크로 사용)
          // IP_ADDR0~3: 예) 192, 168, 1, 100
          // NETMASK_ADDR0~3: 예) 255, 255, 255, 0
          // GW_ADDR0~3: 예) 192, 168, 1, 1
          IP_ADDR4(&ipaddr, IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
          IP_ADDR4(&netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3);
          IP_ADDR4(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
          
          // 네트워크 인터페이스에 정적 IP 주소 설정
          netif_set_addr(netif, &ipaddr, &netmask, &gw);
          // 설정된 정적 IP 주소를 문자열로 변환하여 출력
          ip4addr_ntoa_r(&ipaddr, ip_str, sizeof(ip_str));
          cliPrintf("[!!] DHCP Timeout - Using Static IP: %s\n", ip_str);
        }
      }
    }
    break;

    case DHCP_LINK_DOWN:
    {
      // 이더넷 링크가 끊어진 경우 DHCP 중지
      // DHCP_OFF가 아닌 경우에만 중지 (중복 호출 방지)
      if (DHCP_state != DHCP_OFF)
      {
        // DHCP 클라이언트 중지
        // 내부적으로 DHCP 타이머 정지 및 할당된 IP 해제
        dhcp_stop(netif);
        
        // 상태를 OFF로 전환
        DHCP_state = DHCP_OFF;
        
        cliPrintf("[  ] DHCP stopped (link down)\n");
      }
    }
    break;

    default: 
      break;
  }
}

/**
  * @brief  DHCP 주기적 체크
  * @param  netif: 네트워크 인터페이스
  * @details MX_LWIP_Process() (매 루프마다 호출) -> 500ms 주기로 DHCP_Process() 호출
  */
void DHCP_Periodic_Handle(struct netif *netif)
{
  /* Fine DHCP periodic process every 500ms */
  if (HAL_GetTick() - DHCPfineTimer >= DHCP_FINE_TIMER_MSECS)
  {
    DHCPfineTimer = HAL_GetTick();
    DHCP_Process(netif);
  }
}
#endif

/**
  * @brief  LwIP 초기화 함수
  */
void MX_LWIP_Init(void)
{
  ip_addr_t ipaddr;
  ip_addr_t netmask;
  ip_addr_t gw;

  /* Initialize the LwIP stack without RTOS */
  lwip_init();

  /* IP addresses initialization with DHCP (IPv4) */
  ip_addr_set_zero_ip4(&ipaddr);
  ip_addr_set_zero_ip4(&netmask);
  ip_addr_set_zero_ip4(&gw);

  /* Add the network interface (IPv4/IPv6) without RTOS */
  netif_add(&gnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &netif_input);

  /* Registers the default network interface */
  netif_set_default(&gnetif);

  /* We must always bring the network interface up connection or not... */
  netif_set_up(&gnetif);

  /* Set the link callback function, this function is called on change of link status*/
  netif_set_link_callback(&gnetif, ethernet_link_status_updated);

  /* Start DHCP negotiation for a network interface (IPv4) */
#if LWIP_DHCP
  DHCP_state = DHCP_START;
#endif

  cliPrintf("[  ] LwIP initialized\n");
}

/**
 * @brief  LwIP 스택 초기화 함수
 * 
 * @note 이 함수는 main() 함수에서 HAL_Init() 이후 한 번만 호출됩니다
 * 
 * @details
 * 수행 작업:
 * 1. LwIP 프로토콜 스택 초기화
 * 2. 네트워크 인터페이스 추가 (이더넷)
 * 3. 네트워크 인터페이스 활성화
 * 4. 링크 상태 변경 콜백 등록
 * 5. DHCP 프로세스 시작
 */
void MX_LWIP_Process(void)
{
  /* Read a received packet from the Ethernet buffers */
  ethernetif_input(&gnetif);

  /* Handle timeouts */
  sys_check_timeouts();

#if LWIP_NETIF_LINK_CALLBACK
  /* Handle Ethernet link status */
  Ethernet_Link_Periodic_Handle(&gnetif);
#endif

#if LWIP_DHCP
  /* Handle DHCP state machine */
  DHCP_Periodic_Handle(&gnetif);
#endif
}
