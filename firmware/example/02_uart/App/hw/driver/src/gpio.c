#include "gpio.h"

#ifdef _USE_HW_GPIO

typedef struct
{
  GPIO_TypeDef *port;
  uint32_t      pin;
  uint8_t       mode;
  GPIO_PinState on_state;
  GPIO_PinState off_state;
  bool          init_value;
  const char   *p_name;
} gpio_tbl_t;

/**
 * @brief GPIO 설정 : PC13, OUTPUT, LED ON -> HIGH, 처음부터 HIGH로 시작
 */
const gpio_tbl_t gpio_tbl[HW_GPIO_MAX_CH] =
{
  {GPIOC, GPIO_PIN_13, _DEF_OUTPUT, GPIO_PIN_SET, GPIO_PIN_RESET, _DEF_HIGH, "LED"}, // PC13번 -> LED핀
};


#ifdef _USE_HW_CLI
static void cliGpio(cli_args_t *args);
#endif


bool gpioInit(void)
{
  bool ret = true;

  GPIO_InitTypeDef GPIO_InitStruct = {0};

#ifdef _USE_HW_CLI
  cliAdd("gpio", cliGpio);
#endif

  // 모든 gpio 초기화
  for (int i = 0; i < HW_GPIO_MAX_CH; i++)
  {
    GPIO_InitStruct.Pin = gpio_tbl[i].pin;

    switch (gpio_tbl[i].mode)
    {
      case _DEF_INPUT:
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        break;
      case _DEF_INPUT_PULLUP:
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        break;
      case _DEF_INPUT_PULLDOWN:
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_PULLDOWN;
        break;
      case _DEF_OUTPUT:
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        break;
    }

    HAL_GPIO_Init(gpio_tbl[i].port, &GPIO_InitStruct);

    // 초기값 설정
    if (gpio_tbl[i].mode == _DEF_OUTPUT ||
        gpio_tbl[i].mode == _DEF_OUTPUT_PULLUP ||
        gpio_tbl[i].mode == _DEF_OUTPUT_PULLDOWN)
    {
      gpioPinWrite(i, gpio_tbl[i].init_value);
    }
  }

  return ret;
}

bool gpioPinMode(uint8_t ch, uint8_t mode)
{
  bool             ret             = true;
  GPIO_InitTypeDef GPIO_InitStruct = {0};


  if (ch >= HW_GPIO_MAX_CH)
  {
    return false;
  }

  switch (mode)
  {
    case _DEF_INPUT:
      GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      break;

    case _DEF_INPUT_PULLUP:
      GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
      GPIO_InitStruct.Pull = GPIO_PULLUP;
      break;

    case _DEF_INPUT_PULLDOWN:
      GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
      GPIO_InitStruct.Pull = GPIO_PULLDOWN;
      break;

    case _DEF_OUTPUT:
      GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      break;

    case _DEF_OUTPUT_PULLUP:
      GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
      GPIO_InitStruct.Pull = GPIO_PULLUP;
      break;

    case _DEF_OUTPUT_PULLDOWN:
      GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
      GPIO_InitStruct.Pull = GPIO_PULLDOWN;
      break;
  }

  GPIO_InitStruct.Pin = gpio_tbl[ch].pin;
  HAL_GPIO_Init(gpio_tbl[ch].port, &GPIO_InitStruct);

  return ret;
}

void gpioPinWrite(uint8_t ch, bool value)
{
  if (ch >= HW_GPIO_MAX_CH)
  {
    return;
  }

  if (value)
  {
    HAL_GPIO_WritePin(gpio_tbl[ch].port, gpio_tbl[ch].pin, gpio_tbl[ch].on_state);
  }
  else
  {
    HAL_GPIO_WritePin(gpio_tbl[ch].port, gpio_tbl[ch].pin, gpio_tbl[ch].off_state);
  }
}

bool gpioPinRead(uint8_t ch)
{
  bool ret = false;

  if (ch >= HW_GPIO_MAX_CH)
  {
    return false;
  }

  if (HAL_GPIO_ReadPin(gpio_tbl[ch].port, gpio_tbl[ch].pin) == gpio_tbl[ch].on_state)
  {
    ret = true;
  }

  return ret;
}

void gpioPinToggle(uint8_t ch)
{
  if (ch >= HW_GPIO_MAX_CH)
  {
    return;
  }

  HAL_GPIO_TogglePin(gpio_tbl[ch].port, gpio_tbl[ch].pin);
}

#endif
