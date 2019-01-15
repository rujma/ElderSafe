#include "Buzzer.h"
#include "tim.h"

void startSoundAlert(void)
{
	__HAL_TIM_SET_COMPARE(&htim11,TIM_CHANNEL_1,3000);
}

void stopSoundAlert(void)
{
	__HAL_TIM_SET_COMPARE(&htim11,TIM_CHANNEL_1,0);
}

void buzzer_Init(void)
{
	HAL_TIM_PWM_Start(&htim11,TIM_CHANNEL_1);
	
}
