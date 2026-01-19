#include "buzzer.h"
#include "pwm.h"
#ifdef _USE_HW_CLI
#include "cli.h"
#endif

// BUZZER가 PWM 채널 몇 번인지 명확히 선언
#define BUZZER_PWM_CH 0  // pwm_tbl에서 0번
#define BUZZER_PWM_MAX pwmGetMax(BUZZER_PWM_CH)

static bool is_on = false;
static uint32_t current_freq = 4000;
static uint16_t current_duty = 50; // 50% duty(0~MAX)

#ifdef _USE_HW_CLI
static void cliBuzzer(cli_args_t *args);
#endif

void buzzerInit(void)
{
  is_on = false;
  pwmWrite(BUZZER_PWM_CH, 0);
#ifdef _USE_HW_BUZZER
  cliAdd("buzzer", cliBuzzer);
#endif
}

void buzzerOn(void)
{
  pwmWrite(BUZZER_PWM_CH, current_duty);
  is_on = true;
}

void buzzerOff(void)
{
  pwmWrite(BUZZER_PWM_CH, 0);
  is_on = false;
}

bool buzzerIsOn(void)
{
  return is_on;
}

void buzzerBeep(uint32_t ms)
{
  buzzerOn();
  uint32_t pre = millis();
  while(millis() - pre < ms)
  {

  }
  buzzerOff();
}

// 주파수, 듀티 동적으로 변경
void buzzerTone(uint32_t freq, uint16_t duty)
{
  // 예시: PWM PSC/ARR 계산 후 설정(CubeMX에서 AutoReload Preload, PSC Runtime 변경 허용해야 함)
  // 만약 pwm 드라이버 내에 arr/psc 변경 함수가 있다면 활용
  // 아래는 일반적인 ARR, PSC 계산
  extern TIM_HandleTypeDef htim1; // 실제 사용할 타이머로 변경
  uint32_t timer_clk = 250000000; // APB1=250MHz라 가정
  uint32_t arr = 999;
  uint32_t psc = ((timer_clk / (freq * (arr+1))) - 1);
  __HAL_TIM_SET_PRESCALER(&htim1, psc);
  __HAL_TIM_SET_AUTORELOAD(&htim1, arr);
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, duty);
  current_freq = freq;
  current_duty = duty;
}

#ifdef _USE_HW_BUZZER

#ifdef _USE_HW_CLI
void cliBuzzer(cli_args_t *args)
{
  bool ret = false;

  if (args->argc == 1 && args->isStr(0, "info"))
  {
    cliPrintf("buzzer is_on      : %s\n", buzzerIsOn() ? "ON" : "OFF");
    cliPrintf("buzzer freq/duty  : %lu Hz / %u \n", current_freq, current_duty);
    ret = true;
  }
  else if (args->argc == 2 && args->isStr(0,"test"))
  {
    uint32_t beep_time = (uint32_t)args->getData(1); // ex: 200(ms)
    cliPrintf("buzzer beep %d ms\n", beep_time);
    buzzerBeep(beep_time);
    ret = true;
  }
  else if (args->argc == 3 && args->isStr(0, "tone"))
  {
    uint32_t freq = (uint32_t)args->getData(1);
    uint16_t duty = (uint16_t)args->getData(2);
    cliPrintf("buzzer tone %lu Hz / duty %u \n", freq, duty);
    buzzerTone(freq, duty);
    buzzerOn();
    ret = true;
  }
  else if (args->argc == 1 && args->isStr(0, "off"))
  {
    buzzerOff();
    cliPrintf("buzzer Off\n");
    ret = true;
  }

  if (!ret)
  {
    cliPrintf("buzzer info\n");
    cliPrintf("buzzer test beep_ms\n");
    cliPrintf("buzzer tone freq[Hz] duty[0~1000]\n");
    cliPrintf("buzzer off\n");
  }
}
#endif

#endif
