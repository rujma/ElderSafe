#ifndef __ACCELEROMETER_H_
#define __ACCELEROMETER_H_
#define ALPHA_FILTER 0.8
#define CONVERSION_CONSTANT 0.00390625
typedef struct
{
	double Ax;
	double Ay;
	double Az;
}tAccel;

typedef struct
{
	double A_svm;
	double Theta;
}tAccelParameters;
	
void accel_Init(void);
void accel_Config(void);
tAccel convertToGForce(unsigned char* values);
tAccelParameters calculateParameters(tAccel accel);
tAccel filter_accel_data(tAccel accel);
void accel_Write(unsigned char pin, unsigned char reg);
void accel_Init_GPIO(void);
void accel_Init_DMA(void);
void accel_Init_I2C(void);
void accel_start_DMA(void);
void accel_read_DMA(unsigned char* buffer);


#endif
