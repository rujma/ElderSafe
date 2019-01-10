#include "Buzzer.h"
#include "tim.h"

void startSoundAlert(void)
{
	HAL_TIM_PWM_Start(&htim11,TIM_CHANNEL_1);
}

void stopSoundAlert(void)
{
	HAL_TIM_PWM_Stop(&htim11,TIM_CHANNEL_1);
}

void buzzer_Init(void)
{
	MX_TIM11_Init();
	__HAL_TIM_SET_COMPARE(&htim11,TIM_CHANNEL_1,100);
}
