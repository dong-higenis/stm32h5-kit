#include "modbus_tcp.h"
#include "lwip/tcp.h"
#include <string.h>

/**
 * @brief 통신 포트 및 최대 데이터 패킷 수 정의
 */
#define MODBUS_TCP_PORT         502
#define MAX_MODBUS_DATA_BYTE    100

/**
 * @brief 응답 데이터 프레임 바이트 정의
 */
#define MBAP_HEADER_BYTE              7
#define FUNC_CODE_BYTE                1
#define BYTE_COUNT_BYTE               1

#define MIN_RESPONSE_PACKET_NUM       9

/**
 * @brief 센서 수: 예) 센서 2개 ->  데이터 갯수 2개..
 */
#define SENSOR_NUM                    1

// 센서 값 (임시)
uint8_t sensor_val[SENSOR_NUM] = {0,};

// TCP PCB
static struct tcp_pcb *modbus_pcb = NULL;

// tcp
bool modbusTcpInit(void)
{
  // Protocol Control Block 구조체 생성 (소켓같은 개념)
  modbus_pcb = pcb_new();

  // 널 포인터 체크
  if (modbus_pcb == NULL)
  {
    return false;
  }
  
  // 모든 IP에 대해 바인딩
  err_t bind_result = tcp_bind(modbus_pcb ,IP_ADDR_ANY, MODBUS_TCP_PORT);

  // 에러가 없다면
  if (bind_result == ERR_OK)
  {
    // 상대방의 접속을 대기하기 위해 가벼운 (경량) pcb로 교체한다.
    modbus_pcb = tcp_listen(modbus_pcb);

    // 클라이언트의 접속을 처리 하기 위한 함수 포인터 콜백을 등록한다.
    tcp_accept(modbus_pcb, modbusAccept); 
  }

  return true;
}

static bool modbusTcpResponse(struct tcp_pcb *tpcb, uint8_t *req, uint8_t req_len)
{
  uint16_t transaction_id = (req[0] << 8) | req[1];
  uint16_t protocol_id    = (req[2] << 8) | req[3];
  uint16_t length         = (req[4] << 8) | req[5];
  uint8_t  unit_id        = req[6];
  uint8_t  func_code      = req[7];

  // 고정 값이 맞는지 확인
  if (protocol_id != 0x00)
  {
    return false;
  }

  // 레지스터 read모드가 맞는지 확인
  if (func_code != 0x03)
  {
    return false;
  }

  uint16_t read_start_addr = (req[8] << 8) | req[9];
  uint16_t read_req_length = (req[10] << 8) | req[11];

  // read 길이 범위 초과 (초기 주소 + read_req_len이 max값을 초과하면 안됀다.)
  if (read_start_addr + read_req_length > MAX_MODBUS_DATA_BYTE)
  {
    // 에러 패킷 만들기
    uint8_t err_response[MIN_RESPONSE_PACKET_NUM] = {0,};

    err_response[0] = (transaction_id >> 8);
    err_response[1] = transaction_id;
    err_response[2] = 0x00;
    err_response[3] = 0x00;
    err_response[4] = 0x00;
    err_response[5] = 0x03;
    err_response[6] = 0x00;
    err_response[7] = 0x01;
    err_response[8] = func_code & 0x80;
   
    // 에러 패킷 dataptr에 저장
    tcp_write(tpcb ,err_response ,MIN_RESPONSE_PACKET_NUM, TCP_WRITE_FLAG_COPY);
    
    // 송신
    tcp_output(tpcb);

    return false;
  }

  // 응답 패킷 만들기
  uint8_t modbus_response[MBAP_HEADER_BYTE + FUNC_CODE_BYTE + MAX_MODBUS_DATA_BYTE] = {0,};

  modbus_response[0] = (transaction_id >> 8);
  modbus_response[1] = transaction_id;
  modbus_response[2] = 0x00;
  modbus_response[3] = 0x00;
  
  uint16_t    = SENSOR_NUM;

  modbus_response[4] = 0x00;
  modbus_response[5] = 0x00;

  modbus_response[6] = func_code;
  modbus_response[7] = data_resp_length;

  for (int i=0; i<data_resp_length; i++)
  {
    modbus_response[8 + i] =  sensor_val[i];
  }
  
}