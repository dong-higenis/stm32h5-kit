#include "custom_fram.h"
#include "cli.h" 

static bool    is_fram_inited      = false; // 이곳으로 옮김

#define I2C_FRAM_ADDRESS            0x50 
#define I2C_FRAM_READ_START_ADDRESS 0x00 

extern I2C_HandleTypeDef hi2c4;          

bool customFramInit(void)
{
  uint8_t           read_data;
  bool              ret = false;
  HAL_StatusTypeDef i2c_ret;

  i2c_ret = HAL_I2C_Mem_Read(&hi2c4, (uint16_t)(I2C_FRAM_ADDRESS << 1), I2C_FRAM_READ_START_ADDRESS, I2C_MEMADD_SIZE_8BIT, &read_data, 1, 100);

  if (i2c_ret == HAL_OK)
  {
    ret = true;
    is_fram_inited = true; // 이곳으로 옮김
    cliPrintf("read_data : %02X\n", read_data);
  }

  return ret;
}

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

bool customFramRead(uint16_t addr, uint8_t *data)
{
  HAL_StatusTypeDef i2c_ret;

  i2c_ret = HAL_I2C_Mem_Read(&hi2c4, (uint16_t)(I2C_FRAM_ADDRESS << 1), addr, I2C_MEMADD_SIZE_8BIT, data, 1, 100);

  return (i2c_ret == HAL_OK);
}

/**
 * @brief fram 테스트
 *        사용법 : fram info, 
 *                 fram write 0 0xFF
 *                 fram read  0 
 */
void cliFram(cli_args_t *args)
{
  uint32_t addr;
  uint32_t data;
  uint8_t  read_data;

  if (args->argc == 1 && args->isStr(0, "info"))
  {
    if (is_fram_inited)
    {
      cliPrintf("FRAM ready\n");
    }
    else 
    {
      cliPrintf("FRAM failed\n");
    }
  }
  else if (args->argc == 2 && args->isStr(0, "read"))
  {
    addr = args->getData(1);

    if (customFramRead(addr, &read_data))
    {
      cliPrintf("addr 0x%04X -> 0x%02X\n", addr, read_data);
    }
    else
    {
      cliPrintf("read fail\n");
    }
  }
  else if (args->argc == 3 && args->isStr(0, "write"))
  {
    addr = args->getData(1);
    data = args->getData(2);

    if (customFramWrite(addr, (uint8_t)data))
    {
      cliPrintf("write OK\n");
    }
    else
    {
      cliPrintf("write fail\n");
    }
  }
  else
  {
    cliPrintf("fram info\n");
    cliPrintf("fram read [addr]\n");
    cliPrintf("fram write [addr] [data]\n");
  }
}
