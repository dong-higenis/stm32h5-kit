#ifndef CUSTOM_CAN_H
#define CUSTOM_CAN_H

#include <stdbool.h> // bool 타입 사용을 위해 내장라이브러리 include
#include <fdcan.h> // fdcan 자동생성 파일 include

bool customCanInit(FDCAN_HandleTypeDef *can_handler, bool is_loopback);
void customCanClose(FDCAN_HandleTypeDef *can_handler);

#endif
