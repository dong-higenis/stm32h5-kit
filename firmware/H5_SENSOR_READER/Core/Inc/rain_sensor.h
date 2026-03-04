#ifndef RAIN_SENSOR_H
#define RAIN_SENSOR_H

#include <stdint.h>

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
} rain_sensor_req_t;

typedef struct __attribute__((packed))
{
    uint8_t addr;
    uint8_t func;
    uint8_t byte_count;
    uint16_t rain_val;
    uint16_t crc;
} rain_sensor_resp_t;

void rainSensorInit(rain_sensor_req_t* req);

#ifdef __cplusplus
}
#endif

#endif