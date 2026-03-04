#include "rain_sensor.h"

#define RAIN_SENSOR_ADDR        0x02
#define RAIN_SENSOR_FUNC_READ   0x03
#define RAIN_SENSOR_REG_RAIN    0x0003
#define RAIN_SENSOR_LEN         1

void rainSensorInit(rain_sensor_req_t* req)
{
  req->addr = RAIN_SENSOR_ADDR;
  req->func = RAIN_SENSOR_FUNC_READ;
  req->reg_origin = __builtin_bswap16(RAIN_SENSOR_REG_RAIN);
  req->length = __builtin_bswap16(RAIN_SENSOR_LEN);
}
