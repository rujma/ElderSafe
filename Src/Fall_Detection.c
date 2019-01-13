#include "Accelerometer.h"
#include "math.h"
#include <stddef.h>
#include <stdlib.h>
#include "rtwtypes.h"
#include "predict_fall_types.h"
#include "predict_fall.h"
#include "usart.h"
#include "string.h"

#define MAX_BUFFER 120
#define A_SVM_THRESHOLD 1.5
#define TRUE 1
#define FALSE 0
#define FALL_SAMPLES 30
typedef enum {FALL ,NO_FALL} Fall_Detection_Typedef;

static uint8_t predictions[FALL_SAMPLES];
static char predictions_index;
static char falls = 0;
static char walk = 0;
static char jump = 0;
static char ld = 0;


Fall_Detection_Typedef checkFallEvent()
{
	int value;
	char message[20];
	HAL_UART_Transmit(&huart6,(uint8_t *)"start\r\n",strlen("start\r\n"),100);
	for(int i = 0; i < FALL_SAMPLES; i++)
	{
		value = predictions[i];
		sprintf(message,"%d",value);
		HAL_UART_Transmit(&huart6,(uint8_t *)message,strlen(message),100);
		switch(value)
		{
			case 0:
				falls++;
				break;
			case 1:
				walk++;
				break;
			case 2:
				jump++;
				break;
			case 3:
				ld++;
				break;
		}
	}
	HAL_UART_Transmit(&huart6,(uint8_t *)"\r\nend\r\n",strlen("\r\nend\r\n"),100);
	if(falls > walk && falls > jump && falls > ld) 
	{
		falls = 0;
		walk = 0;
		jump = 0;
		ld = 0;
		return FALL;
	}
		falls = 0;
		walk = 0;
		jump = 0;
		ld = 0;
	return NO_FALL;
}

Fall_Detection_Typedef fall_detection(tAccelParameters param)
{
	static _Bool fall_started;
	if(!fall_started)
	{
		if(param.A_svm > A_SVM_THRESHOLD)
		{
			fall_started = TRUE;
		}
	}
	else
	{
		predictions[predictions_index] = predict_fall(param.A_svm, param.Theta);
		predictions_index++;
		if(predictions_index == FALL_SAMPLES)
		{
			predictions_index = 0;
			fall_started = FALSE;
			if(checkFallEvent() == FALL) 
			{
				return FALL;
			}
		}
		return NO_FALL;
	}
		return NO_FALL;
}





