#ifndef __FALL_DETECTION_H_
#define __FALL_DETECTION_H_
#include "Accelerometer.h"
typedef enum {FALL,NO_FALL} Fall_Detection_Typedef;
Fall_Detection_Typedef fall_detection(tAccelParameters param);
#endif
