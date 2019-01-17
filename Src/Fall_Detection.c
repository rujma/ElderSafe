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
#define A_SVM_THRESHOLD 1.4
#define TRUE 1
#define FALSE 0
#define FALL_SAMPLES 30
typedef enum {FALL,NO_FALL} Fall_Detection_Typedef;


static uint8_t predictions[FALL_SAMPLES];
static char predictions_index;
static char falls = 0;
static char walk = 0;
static char jump = 0;
static char ld = 0;

Fall_Detection_Typedef checkFallEvent()
{
	int value;
	Fall_Detection_Typedef fall_flag = NO_FALL;
	// Iterate through the predictions array
	for(int i = 0; i < FALL_SAMPLES; i++)
	{
		value = predictions[i];
		// Count the number of occurrences of each class
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
	// If the FALL class has the most occurrences, it's a fall
	if(falls > walk && falls > jump && falls > ld)  
		fall_flag = FALL;
	else 
		fall_flag = NO_FALL;
	
	//Reset prediction counters
	falls = 0;
	walk = 0;
	jump = 0;
	ld = 0;
	// Return fall status													
	return fall_flag;
}

Fall_Detection_Typedef fall_detection(tAccelParameters param)
{
	static _Bool fall_started;
	if(!fall_started)
	{
		// Resulting force is above threshold
		if(param.A_svm > A_SVM_THRESHOLD)
		{
			// Signal the start of a fall
			fall_started = TRUE;
		}
	}
	else
	{
		// Fall is occuring: predict the class of next resulting force and pitch
		predictions[predictions_index] = predict_fall(param.A_svm, param.Theta);
		predictions_index++;
		// Buffer is full, ready to count the occurrences
		if(predictions_index == FALL_SAMPLES)
		{
			predictions_index = 0;
			fall_started = FALSE;
			if(checkFallEvent() == FALL) 
			{
				HAL_UART_Transmit_DMA(&huart6,(uint8_t *)"fall\r\n",strlen("fall\r\n"));
				return FALL;
				
			}
			else HAL_UART_Transmit_DMA(&huart6,(uint8_t *)"no\r\n",strlen("no\r\n"));
		}
	}
	return NO_FALL;
}





