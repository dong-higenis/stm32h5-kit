#include "wind_sensor.h"


#define SLAVE_ADDR                   0x0A
#define WIND_SENSOR_FUNC_READ        0x03
#define WIND_SENSOR_FUNC_WRITE       0x06
#define WIND_SENSOR_REG_SPEED        0x0000
#define WIND_SENSOR_REG_SET_ADDR     0x0100
#define WIND_SENSOR_REG_SET_BAUD     0x0101
#define WIND_SENSOR_FRAME_SIZE       8
#define WIND_SENSOR_RESP_SIZE        7

typedef enum
{
  BAUD_2400,
  BAUD_4800,
  BAUD_9600
} wind_sensor_baud;

static uint8_t slave_address = SLAVE_ADDR;

/**
 * @brief modbus 사용을 위해 big-endian 으로 바꾸기
 */
void windSensorInit(wind_sensor_req_t* req)
{
  req->addr       = slave_address;
  req->func       = WIND_SENSOR_FUNC_READ;
  req->reg_origin = __builtin_bswap16(WIND_SENSOR_REG_SPEED);
  req->length     = __builtin_bswap16(1); // 레지스터 한개 읽기
}

/**
 * @brief slave address를 변경하는 함수
 *        패킷: [prev_addr] [06] [01] [00] [target_id] [target_id]
 *        연속 두 번 전송해야 함
 *
 * @param req      요청 구조체
 * @param new_addr 변경할 새 슬레이브 주소
 */
bool windSensorSetAddr(wind_sensor_req_t* req, uint8_t new_addr)
{
  req->addr       = slave_address;
  req->func       = WIND_SENSOR_FUNC_WRITE;
  req->reg_origin = __builtin_bswap16(WIND_SENSOR_REG_SET_ADDR);
  req->length     = (uint16_t)(new_addr << 8) | new_addr;

  slave_address = new_addr;

  return true;
}

/**
 * @brief 센서의 baudrate를 설정하는 함수
 */
bool windSensorSetBaud(wind_sensor_req_t* req, uint32_t baud)
{
  bool ret;

  uint8_t new_baud;

  switch (baud)
  {
    case 2400:
      ret = true;
      new_baud = BAUD_2400;
      break;    
    case 4800:
      ret = true;
      new_baud = BAUD_4800;
      break;  
    case 9600:
      ret = true;
      new_baud = BAUD_9600;
      break;
      
    default:
      ret = false;
      break;
  }
  req->addr       = slave_address;
  req->func       = WIND_SENSOR_FUNC_WRITE;
  req->reg_origin = __builtin_bswap16(WIND_SENSOR_REG_SET_BAUD);
  req->length     = (uint16_t)(new_baud << 8) | new_baud;

  return ret;
}
