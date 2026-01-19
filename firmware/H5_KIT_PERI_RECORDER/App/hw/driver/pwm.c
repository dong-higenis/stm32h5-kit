#include "pwm.h"


#ifdef _USE_HW_PWM
#include "cli.h"

#define NAME_DEF(x)  x, #x // 상수 , 문자열



typedef struct
{
  TIM_HandleTypeDef *p_htim;
  uint16_t           max_value;
  PwmPinName_t       pin_name;
  const char        *p_name;
} pwm_tbl_t;


#ifdef _USE_HW_PWM
static void cliPwm(cli_args_t *args);
#endif

static bool is_init = false;

extern TIM_HandleTypeDef htim1;


static uint16_t pwm_duty[PWM_MAX_CH] = {0, };

static const pwm_tbl_t pwm_tbl[PWM_MAX_CH] =
{
    {&htim1, MAX_PWM_VALUE, NAME_DEF(BUZZER_PWM)}, // *p_htim, max_value, pin_name
};


static bool pwmInitTimer(void);

bool pwmInit(void)
{
  bool ret;

  for (int i=0; i<PWM_MAX_CH; i++)
  {
    assert(i == (int)pwm_tbl[i].pin_name); // 조건 거짓 == 오류 프로그램 중단
  }


  ret = pwmInitTimer();
  is_init = ret;
  if(is_init)
  {
    logPrintf("[%s] pwmInit()\n", is_init ? "OK":"E_");
  }

#ifdef _USE_HW_PWM
  cliAdd("pwm", cliPwm);
#endif
  return ret;
}

bool pwmInitTimer(void)
{
  bool ret = true;

  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);

  return ret;
}

bool pwmIsInit(void)
{
  return is_init;
}

void pwmWrite(uint8_t ch, uint16_t pwm_data)
{
  assert(ch < PWM_MAX_CH);
  assert(pwm_data <= pwm_tbl[ch].max_value);

  pwm_duty[ch] = pwm_data;

  switch(ch)
  {
    case _DEF_PWM1:
      htim1.Instance->CCR1 = pwm_duty[ch];
      break;
  }
}

uint16_t pwmRead(uint8_t ch)
{
  if (ch >= HW_PWM_MAX_CH) return 0;

  return pwm_duty[ch];
}

uint16_t pwmGetMax(uint8_t ch)
{
  if (ch >= HW_PWM_MAX_CH) return MAX_PWM_VALUE;

  return pwm_tbl[ch].max_value;
}


#ifdef _USE_HW_PWM
void cliPwm(cli_args_t *args)
{
  bool ret = false;

  if (args->argc == 1 && args->isStr(0, "info"))
  {
    for (int i = 0; i < PWM_MAX_CH; i++)
    {
      cliPrintf("%02d. %-32s : %03d/%03d %-2d%%\n",
                i,
                pwm_tbl[i].p_name,
                pwm_duty[i],
                pwm_tbl[i].max_value,
                pwm_duty[i] * 100 / pwm_tbl[i].max_value);
    }
    ret = true;
  }

//
//  if (args->argc == 3 && args->isStr(0, "set"))
//  {
//    ch  = (uint8_t)args->getData(1);
//    pwm = (uint32_t)args->getData(2);
//
//    ch = constrain(ch, 0, PWM_MAX_CH);
//
//    pwmWrite(ch, pwm);
//    cliPrintf("pwm ch%d %d\n", ch, pwm);
//    ret = true;
//  }
//
//  if (args->argc == 2 && args->isStr(0, "get"))
//  {
//    ch = (uint8_t)args->getData(1);
//
//    cliPrintf("pwm ch%d %d\n", ch, pwmRead(ch));
//    ret = true;
//  }

  if (ret == false)
  {
    cliPrintf("pwm info\n");
  }

}
#endif

#endif
