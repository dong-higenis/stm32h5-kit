#include "fram.h"
#include "cli.h"
#include "i2c.h"

#ifdef _USE_HW_FRAM


#ifdef _USE_HW_CLI
void cliFram(cli_args_t *args);
#endif


#define FRAM_MAX_SIZE         HW_FRAM_MAX_SIZE

static bool framWritePage(uint32_t addr, uint8_t* data_in, uint32_t len);
static bool    is_init  = false;
static uint8_t i2c_ch   = HW_I2C_CH_FRAM;
static uint8_t i2c_addr = 0x50;

bool framInit()
{
  bool ret;

  ret = i2cBegin(i2c_ch, 400);

  if (ret == true)
  {
    ret = framValid(0x00);
  }

  logPrintf("[%s] framInit()\n", ret ? "OK" : "NG");
  if (ret == true)
  {
    logPrintf("     addr : 0x%02X\n", i2c_addr);
  }
#ifdef _USE_HW_CLI
  cliAdd("fram", cliFram);
#endif

  is_init = ret;

  return ret;
}

bool framIsInit(void)
{
  return is_init;
}

bool framValid(uint32_t addr)
{
  uint8_t data;
  bool    ret;

  if (addr >= FRAM_MAX_SIZE)
  {
    return false;
  }

  ret = i2cReadBytes(i2c_ch, i2c_addr, addr, &data, 1, 100);

  return ret;
}

bool framReadByte(uint32_t addr, uint8_t *p_data)
{
  bool ret;

  if (addr >= FRAM_MAX_SIZE)
  {
    return false;
  }
  //(256) > 0x100  i s dev_addr = i2c_Addr(0x50) | (0x01 addr>>8 0x100 > 0x01)
  // 0x51
  // 0x100
  uint8_t dev_addr = i2c_addr | ((addr>>8)&0x3);
  ret = i2cReadBytes(i2c_ch, dev_addr, (addr&0xFF), p_data, 1, 100);

  return ret;
}

bool framWriteByte(uint32_t addr, uint8_t data_in)
{
  uint32_t pre_time;
  bool     ret = false;

  if (addr >= FRAM_MAX_SIZE)
  {
    return false;
  }

  uint8_t dev_addr = i2c_addr | ((addr>>8)&0x3);
  ret = i2cWriteBytes(i2c_ch, dev_addr, (addr&0xFF), &data_in, 1, 10);

  pre_time = millis();
  while (millis() - pre_time < 100)
  {
    ret = i2cIsDeviceReady(i2c_ch, i2c_addr);
    if (ret == true)
    {
      break;
    }
    delay(1);
  }
  return ret;
}

static bool framWritePage(uint32_t addr, uint8_t *data_in, uint32_t len)
{
  bool ret = false;

  if (addr >= FRAM_MAX_SIZE)
  {
    return false;
  }

  uint8_t dev_addr = i2c_addr | ((addr>>8)&0x3);
  ret = i2cWriteBytes(i2c_ch, dev_addr, addr, data_in, len, 10);

  return ret;
}

bool framRead(uint32_t addr, uint8_t *p_data, uint32_t length)
{
  bool     ret = true;
  uint32_t i;

  for (i = 0; i < length; i++)
  {
    ret = framReadByte(addr + i, &p_data[i]);
    if (ret != true)
    {
      break;
    }
  }

  return ret;
}

bool framWrite(uint32_t addr, uint8_t *p_data, uint32_t length)
{
  bool     ret = false;
  uint32_t i;


  for (i = 0; i < length; i++)
  {
    ret = framWriteByte(addr + i, p_data[i]);
    if (ret == false)
    {
      break;
    }
  }
  return ret;
}

uint32_t framGetLength(void)
{
  return FRAM_MAX_SIZE;
}

bool framFormat(void)
{
  return true;
}


#ifdef _USE_HW_CLI
void cliFram(cli_args_t *args)
{
  bool     ret = true;
  uint32_t i;
  uint32_t addr;
  uint32_t length;
  uint8_t  data;
  uint32_t pre_time;
  bool     fram_ret;


  if (args->argc == 1)
  {
    if (args->isStr(0, "info") == true)
    {
      cliPrintf("fram init   : %d\n", framIsInit());
      cliPrintf("fram length : %d bytes\n", framGetLength());
    }
    else if (args->isStr(0, "format") == true)
    {
      if (framFormat() == true)
      {
        cliPrintf("format OK\n");
      }
      else
      {
        cliPrintf("format Fail\n");
      }
    }
    else if (args->isStr(0, "clear") == true)
    {
      uint32_t len = framGetLength();
      for (i=0; i<len; i++)
      {
        fram_ret = framWriteByte(i, 0xFF);
        if (fram_ret == false)
        {
          cliPrintf("clear Err\n");
          break;
        }
      }
    }
    else
    {
      ret = false;
    }
  }
  else if (args->argc == 3)
  {
    if (args->isStr(0, "read") == true)
    {
      addr   = (uint32_t)args->getData(1);
      length = (uint32_t)args->getData(2);

      if (length > framGetLength())
      {
        cliPrintf("length error\n");
      }

      for (i = 0; i < length; i++)
      {
        if (framReadByte(addr + i, &data) == true)
        {
          cliPrintf("addr : %d\t 0x%02X\n", addr + i, data);
        }
        else
        {
          cliPrintf("framReadByte() Error\n");
          break;
        }
      }
    }
    else  if (args->isStr(0, "write") == true)
    {
      addr = (uint32_t)args->getData(1);
      data = (uint8_t)args->getData(2);

      pre_time = millis();
      fram_ret  = framWriteByte(addr, data);

      cliPrintf("addr : %d\t 0x%02X %dms\n", addr, data, millis() - pre_time);
      if (fram_ret)
      {
        cliPrintf("OK\n");
      }
      else
      {
        cliPrintf("FAIL\n");
      }
    }
    else if (args->isStr(0, "page") && args->isStr(1, "test"))
    {
      addr = (uint32_t)args->getData(2);
      data = (uint8_t)args->getData(3);

      uint8_t data2[2] = {0x12, 0x31};

      framWritePage(addr, data2, 2);

    }
  }
  else
  {
    ret = false;
  }

  if (ret == false)
  {
    cliPrintf("fram info\n");
    cliPrintf("fram format\n");
    cliPrintf("fram read  [addr] [length]\n");
    cliPrintf("fram write [addr] [data]\n");
    cliPrintf("fram page test [addr] [data]\n");
    cliPrintf("fram clear\n");
  }
}

#endif

#endif
