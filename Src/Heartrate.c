#include "Heartrate.h"
#include <stdbool.h>
#include "adc.h"
#include "tim.h"
#include "usart.h"

#define ARM_MATH_CM7
#include "arm_math.h"

static float	lastBPM;
static char tachyCount;
static char bradyCount;

static int peakSearcher;
static int peakCounter;
static int hrPeak[2];

// Filter variables
static float32_t ecgInput[HR_SAMPLES];
static int ecgDataCount;
static float32_t ecgOutput[HR_SAMPLES];
static float32_t firStateF32[FIR_BLOCK_SIZE + NUM_TAPS - 1];
static arm_fir_instance_f32 S;
static float32_t *inputF32, *outputF32;
static uint32_t blockSize;
static uint32_t numBlocks;
static const float32_t firCoeffs[NUM_TAPS] = 
	{  0.0015f,  0.0022f,  0.0033f,  0.0044f,  0.0047f,  0.0030f, -0.0022f, 
		-0.0118f, -0.0262f, -0.0447f, -0.0656f, -0.0863f, -0.1038f,
		-0.1157f,  0.8788f, -0.1157f, -0.1038f, -0.0863f, -0.0656f,
		-0.0447f, -0.0262f, -0.0118f, -0.0022f,  0.0030f,  0.0047f,
		0.0044f,  0.0033f,  0.0022f,  0.0015f 
};


void HR_Init()
{	
	ecgDataCount = 0;
	peakSearcher = 0;
	peakCounter = 0;
	hrPeak[0] = 0;
	hrPeak[1] = 0;
	// Initialize filter
	blockSize = FIR_BLOCK_SIZE;
	numBlocks = HR_SAMPLES / FIR_BLOCK_SIZE;
	inputF32 = &ecgInput[0];
	outputF32 = &ecgOutput[0];
	arm_fir_init_f32(&S, NUM_TAPS, (float32_t *)&firCoeffs[0], &firStateF32[0], blockSize);
	// Initialize peripherals
	HAL_ADC_Start_IT(&hadc1);
	HAL_TIM_Base_Start(&htim6);
}

_Bool storeECGData(int data)
{
	ecgInput[ecgDataCount++] = data;
	// Signal full buffer
	if(ecgDataCount == HR_SAMPLES){
		ecgDataCount = 0;
		return true;
	}
	// Buffer still not full
	return false;
}

void firFilter()
{
	for(int i=0; i < numBlocks; i++)
  {
    arm_fir_f32(&S, inputF32 + (i * blockSize), outputF32 + (i * blockSize), blockSize);
  }
}

_Bool searchBeat()
{
	float hrValue = 0;
	uint32_t hrValueIndex = 0;
	int absoluteIndex;
	hrPeak[0] = 0;
	hrPeak[1] = 0;
	
	for(peakSearcher = 0, peakCounter = 0; peakSearcher < HR_SAMPLES && peakCounter < 2; peakSearcher = peakSearcher + 10)
	{
		// Get the min of that block
		arm_min_f32((float32_t*)ecgOutput + peakSearcher, BLOCK_SIZE, (float32_t*)&hrValue, &hrValueIndex);
		absoluteIndex = hrValueIndex + peakSearcher;
		// Check if min is on beat part of wave
		if(hrValue < FIR_THRESHOLD)
		{
			// If it is, store the index
			hrPeak[peakCounter++] = absoluteIndex;
		}
	}
	return (ignoreOutliers() && !muscleNoise());
}

_Bool muscleNoise()
{
	if(hrPeak[0] + BEAT_OFFSET > hrPeak[1])
		return true;
	return false;
}
_Bool ignoreOutliers()
{
	// Avoid potential lead disconnects
	if(hrPeak[0] == 0 && hrPeak[1] == 0) return false;
	// Error on minimum search
	if(hrPeak[0] > hrPeak[1]) return false;
	// Nothing to ignore
	return true;
}

void calculateHeartRate()
{
	// Heart rate calculation: 60 / ((index offset)*sampling period)
	lastBPM = 60 / ((hrPeak[1] - hrPeak[0])*PERIOD);
}

int getLastHeartRate()
{
	// Return last heart rate value
	return (int)lastBPM;
}

void resetHeartCount()
{
	tachyCount = 0;
	bradyCount = 0;
}
_Bool checkHeartRateIssue(int lastBPM)
{
	if(lastBPM > HRMAX){
		tachyCount += 1;
		if(tachyCount == MAX_H)
			return true;
		return false;
	}
	else
	{
		if(lastBPM < HRMIN)
		{
			bradyCount += 1;
			if(bradyCount == MAX_H)
				return true;
			return false;
		}
	}
	resetHeartCount();
	return false;
}





