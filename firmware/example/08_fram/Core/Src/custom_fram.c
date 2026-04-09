#include "custom_fram.h"
#include "cli.h" // cli를 사용할것이므로 cli include
/**
 * @brief 아래 코드는 초보자용 예제로서, 간단하게 i2c를 이용하여, fram을 read하고, write하게끔 구성되어있습니다.
 *        oled를 다룰때도 i2c를 이용했으나, i2c를 위주로 예제를 설명해두지 않았습니다.
 *        oled는 무언가가 전송되었고, 그 결과가 어떤지만 보면 이해 가능하기 때문입니다.
 *
 * @details 먼저 아래 define문이 뜻하는걸 읽어주시길 바랍니다.
 *          i2c는 동기식 직렬 통신 방식으로서, 2선방식입니다. (scl, sda)
 *          master한명과, 여러명의 슬레이브를 두 선으로 다룰수가 있습니다.
 *
 *          1. master는 특정 slave를 주소로서 지정하게 됩니다.
 *          2. i2c핸들러를 extern하고, 간단히 1바이트를 fram의 시작주소에서 읽어서 출력해보고 Init여부를 판단해 봅시다.
 *          3. 간단히 쓰기, 읽기, 및 ACK 응답 여부 함수를 구현하고 main에서 cli함수를 작성해 줍시다.
 *
 */

#define I2C_FRAM_ADDRESS            0x50 // slave 주소 (fram의 i2c주소)
#define I2C_FRAM_READ_START_ADDRESS 0x00 // fram내 시작 주소 (i2c주소 아님!!)

extern I2C_HandleTypeDef hi2c4;          // main에서 참조한 i2c 핸들러

bool customFramInit(void)
{
  uint8_t           read_data;
  bool              ret = false;
  HAL_StatusTypeDef i2c_ret;

  i2c_ret = HAL_I2C_Mem_Read(&hi2c4, (uint16_t)(I2C_FRAM_ADDRESS << 1), I2C_FRAM_READ_START_ADDRESS, I2C_MEMADD_SIZE_8BIT, &read_data, 1, 100);

  // 위 함수의 의미
  // i2c통신을 통해 fram의 시작 번지를 기준으로 1바이트를 읽어 read_data에 값을 저장, 100ms의 최소한의 블로킹 딜레이를 줌
  // slave의 주소는 7비트가 아닌 8비트 형식으로 보내는게 i2c표준. (따라서 "<< 1" 한칸만큼 밀어서, 비트수를 강제로 맞춰주는것)

  // 이제 아래에 위 함수가 성공했는지를 기준으로 Init 성공/ 실패 여부를 판단 하면 되겠습니다.
  if (i2c_ret == HAL_OK)
  {
    ret = true;
    cliPrintf("read_data : %02X\n", read_data);
  }

  return ret;
}

/**
 * @brief FRAM에 1(uint8_t)바이트 쓰기
 * @param addr FRAM 내부 메모리 주소 (0x00부터 시작)
 * @param data 쓸 데이터 (0x00 ~ 0xFF)
 * @return true: 성공, false: 실패
 */
bool customFramWrite(uint16_t addr, uint8_t data)
{
  HAL_StatusTypeDef i2c_ret;

  i2c_ret = HAL_I2C_Mem_Write(&hi2c4, (uint16_t)(I2C_FRAM_ADDRESS << 1), addr, I2C_MEMADD_SIZE_8BIT, &data, 1, 100);

  if (i2c_ret == HAL_OK)
  {
    return true;
  }

  return false;
}

/**
 * @brief FRAM에서 1바이트를 읽어오는 함수
 */
bool customFramRead(uint16_t addr, uint8_t *data)
{
  HAL_StatusTypeDef i2c_ret;

  i2c_ret = HAL_I2C_Mem_Read(&hi2c4, (uint16_t)(I2C_FRAM_ADDRESS << 1), addr, I2C_MEMADD_SIZE_8BIT, data, 1, 100);

  return (i2c_ret == HAL_OK);
}
