/**
  ******************************************************************************
  * @file    lwip.h
  * @brief   LwIP 초기화 및 관리 함수 헤더 파일
  * @author  dacostal
  * @date    Jun 14, 2024
  ******************************************************************************
  * @attention
  *
  * LwIP 네트워크 스택의 초기화, DHCP 관리, 링크 상태 모니터링을 위한
  * 함수 및 변수 선언을 포함합니다.
  *
  ******************************************************************************
  */

#ifndef __LWIP_H__
#define __LWIP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "lwip/netif.h"



void MX_LWIP_Init(void);
void MX_LWIP_Process(void);
bool lwipIsLink(void);
bool lwipGetIPAddress(char *ip_buffer, size_t size);
const char* lwipGetDHCPStateString(void);
void ethernet_link_status_updated(struct netif *netif);
void Ethernet_Link_Periodic_Handle(struct netif *netif);
void DHCP_Process(struct netif *netif);
void DHCP_Periodic_Handle(struct netif *netif);

#ifdef __cplusplus
}
#endif

#endif 
