#ifndef SSD1306_H_
#define SSD1306_H_

#include <stdint.h>
#include <stdbool.h>
#include "main.h"

// I2C 핸들 (main.c에서 extern으로 가져옴)
extern I2C_HandleTypeDef hi2c1;  // 사용하는 I2C 인터페이스에 맞게 수정

#define SSD1306_I2C_ADDR    0x3C << 1  // I2C 주소 (7비트를 8비트로 변환)
#define SSD1306_WIDTH       128
#define SSD1306_HEIGHT      64

// 초기화 및 기본 함수
bool ssd1306Init(void);
void ssd1306Fill(uint8_t color);
void ssd1306UpdateScreen(void);
void ssd1306SetCursor(uint8_t x, uint8_t y);
void ssd1306WriteString(const char* str);
void ssd1306Clear(void);
void ssd1306WriteChar(char ch);

#endif /* SSD1306_H_ */
