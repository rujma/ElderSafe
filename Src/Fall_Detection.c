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
#define PITCH_THRESHOLD 50
#define TRUE 1
#define FALSE 0
#define FALL_SAMPLES 30
typedef enum {FALL ,NO_FALL} Fall_Detection_Typedef;

static tAccelParameters samples_buffer[MAX_BUFFER];
static unsigned char samples_buffer_index;

static char accelIndex;

double forceArray[30];
double pitchArray[30];
double prediction[30];
static char falls = 0;
static char walk = 0;
static char jump = 0;
static char ld = 0;

int checkFallEvent()
{
	int value;
	char message[20];
	HAL_UART_Transmit(&huart6,(uint8_t *)"start\r\n",strlen("start\r\n"),100);
	for(int i = 0; i < FALL_SAMPLES; i++)
	{
		value = predict_fall(forceArray[i], pitchArray[i]);
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
		return 1;
	}
		falls = 0;
		walk = 0;
		jump = 0;
		ld = 0;
	return 0;
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
		forceArray[accelIndex] = param.A_svm;
		pitchArray[accelIndex] = param.Theta;
		accelIndex++;
		if(accelIndex == FALL_SAMPLES)
		{
			accelIndex = 0;
			fall_started = FALSE;
			if(checkFallEvent() == 1) 
			{
				return FALL;
			}
		}
		return NO_FALL;
	}
		return NO_FALL;
}





