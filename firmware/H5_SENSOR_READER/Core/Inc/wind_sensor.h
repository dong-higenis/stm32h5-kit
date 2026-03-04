#ifndef WIND_SENSOR_H
#define WIND_SENSOR_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __attribute__((packed))
{
    uint8_t addr;
    uint8_t func;
    uint16_t reg_origin;
    uint16_t length;
    uint16_t crc;
} wind_sensor_req_t;

typedef struct __attribute__((packed))
{
    uint8_t addr;
    uint8_t func;
    uint8_t byte_count;
    uint16_t wind_val;
    uint16_t crc;
} wind_sensor_resp_t;

void windSensorInit(wind_sensor_req_t* req);
bool windSensorSetAddr(wind_sensor_req_t* req, uint8_t new_addr);
bool windSensorSetBaud(wind_sensor_req_t* req, uint32_t baud);

#ifdef __cplusplus
}
#endif

#endif