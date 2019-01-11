#ifndef _HEARTRATE_H_
#define _HEARTRATE_H_

#define HRMAX 120
#define HRMIN 40
#define MAX_H 5
#define HR_SAMPLES 128
#define BLOCK_SIZE 10
#define BEAT_OFFSET 20
#define THRESHOLD 400
#define PERIOD 0.02
#define FIR_BLOCK_SIZE 32
#define NUM_TAPS 29
#define FIR_THRESHOLD -75
#define FALSE 0
#define TRUE 1

// Function declaration
void calculateHeartRate(void);
_Bool ignoreOutliers(void);
_Bool searchBeat(void);
int getLastHeartRate(void);
void HR_Init(void);
_Bool checkHeartRateIssue(int lastBPM);
void resetHeartCount(void);
void firFilter(void);
_Bool muscleNoise(void);
_Bool storeECGData(int);



#endif
