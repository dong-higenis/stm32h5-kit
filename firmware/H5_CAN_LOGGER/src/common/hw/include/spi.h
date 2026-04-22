/*
 * spi.h
 *
 *  Created on: 2020. 12. 27.
 *      Author: baram
 */

#ifndef SRC_COMMON_HW_INCLUDE_SPI_H_
#define SRC_COMMON_HW_INCLUDE_SPI_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "hw_def.h"

#ifdef _USE_HW_SPI

#define SPI_MAX_CH HW_SPI_MAX_CH


#define SPI_MODE0  0
#define SPI_MODE1  1
#define SPI_MODE2  2
#define SPI_MODE3  3

/**
 * @brief pin def
 */
#ifdef _USE_HW_SPI
    #define SPI_NSS_PIN   GPIO_PIN_4
    #define SPI_NSS_PORT  GPIOA
    #define SPI_MOSI_PIN  GPIO_PIN_7
    #define SPI_MOSI_PORT GPIOD
    #define SPI_MISO_PIN  GPIO_PIN_6
    #define SPI_MISO_PORT GPIOA
    #define SPI_SCK_PIN   GPIO_PIN_3
    #define SPI_SCK_PORT  GPIOB
    #define SPI_GPIO_CLK_EN()         \
    do                                \
    {                                 \
        __HAL_RCC_GPIOA_CLK_ENABLE(); \
        __HAL_RCC_GPIOB_CLK_ENABLE(); \
        __HAL_RCC_GPIOD_CLK_ENABLE(); \
    } while (0)
#endif


  bool spiInit(void);
  bool spiBegin(uint8_t ch);
  void spiSetBitOrder(uint8_t ch, uint8_t bitOrder);
  void spiSetClockDivider(uint8_t ch, uint32_t clockDiv);
  void spiSetDataMode(uint8_t ch, uint8_t dataMode);
  void spiSetBitWidth(uint8_t ch, uint8_t bit_width);

  bool     spiTransfer(uint8_t ch, uint8_t *tx_buf, uint8_t *rx_buf, uint32_t length, uint32_t timeout);
  uint8_t  spiTransfer8(uint8_t ch, uint8_t data);
  uint16_t spiTransfer16(uint8_t ch, uint16_t data);

  bool spiTransferDMA(uint8_t ch, uint8_t *tx_buf, uint8_t *rx_buf, uint32_t length, uint32_t timeout);

  void spiDmaTxStart(uint8_t ch, uint8_t *p_buf, uint32_t length);
  bool spiDmaTxTransfer(uint8_t ch, void *buf, uint32_t length, uint32_t timeout);
  bool spiDmaTxIsDone(uint8_t ch);
  void spiAttachTxInterrupt(uint8_t ch, void (*func)());


#endif

#ifdef __cplusplus
}
#endif

#endif /* SRC_COMMON_HW_INCLUDE_SPI_H_ */
