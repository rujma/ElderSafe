#include "Temperature.h"
#include "stm32f7xx_hal.h"
#include "cmsis_os.h"
#include "i2c.h"
#include "tim.h"
#include <stdbool.h>

static float lastTemp;
static uint8_t feverCount;
static uint8_t coldCount;
static uint8_t regBuffer[2];

void resetTempCount(void)
{
	feverCount = 0;
	coldCount = 0;
}
int readTemperature()
{
	int temp;
	HAL_I2C_Mem_Read(&hi2c4, SLAVE_ADRESS, OBJ_TEMP, 1, regBuffer, 2, 100);
	temp = (regBuffer[1] << 8 | regBuffer[0]);
	return temp;
}

void calculateTemperature(int temp)
{
	// Calculate the real temperature
	lastTemp = temp * 0.02 - 273.15;
}

float getLastTemperature()
{
	// Return last temperature value
	return lastTemp;
}

// Checks the last temperature value and returns if issue is detected
_Bool checkTemperatureIssue(float lastTemperature)
{
	if(lastTemperature >= TMAX)
	{
		feverCount += 1;
		if(feverCount == MAX_TEMP)
			return true;
		return false;
	}
	else
	{
		if(lastTemperature <= TMIN)
		{
			coldCount += 1;
			if(coldCount == MAX_TEMP)
				return true;
			return false;
		}
	}
	resetTempCount();
	return false;
}

void TEMP_Init()
{
	HAL_TIM_Base_Start_IT(&htim5);
	resetTempCount();
}
