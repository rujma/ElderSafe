#include "Accelerometer.h"
#include "ADXL345.h"
#include "i2c.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#define ARM_MATH_CM7
#include <arm_math.h>
#define FALSE 0
#define TRUE 1

/* dma buffer to read from the accelerometer*/
static  uint8_t dma_buffer[6]; 
/*#######################################################################
name: convertToGForce
parameters:  unsigned char* values - pointer to raw data of the accelerometer
return: struct tAccel : struct with converted values to g force of the 3 axis
#########################################################################*/
tAccel convertToGForce(unsigned char* values)
{
	tAccel accel;
	int16_t ax,ay,az;
	/*join the MSBs to LSBs and multiply by the conversion_constant*/
		ax = ((((int16_t)values[1])<<8)|values[0]);
		ay = ((((int16_t)values[3])<<8)|values[2]);
		az = ((((int16_t)values[5])<<8)|values[4]);
		accel.Ax = ax * CONVERSION_CONSTANT;
		accel.Ay = ay* CONVERSION_CONSTANT;
		accel.Az = az * CONVERSION_CONSTANT;
	
	return accel;
}
/*#######################################################################
name: calculate_A_svm
parameters: tAccel accel- accelerometer g force in 3 axis
return: double : magnitude of force
#########################################################################*/
double calculate_A_svm(tAccel accel)
{
	float32_t src[3] ={(float32_t)accel.Ax,accel.Ay,accel.Az};
	float32_t power_sum,A_svm;
	arm_power_f32(src,3,&power_sum);
	arm_sqrt_f32(power_sum,(float32_t*)&A_svm);
	return A_svm;
}
/*#######################################################################
name: calculate_Theta
parameters: tAccel accel- accelerometer g force in 3 axis
return: double : pitch angle in degrees
#########################################################################*/
double calculate_Theta(tAccel accel)
{
	float32_t src[2] ={(float32_t)accel.Ay,(float32_t)accel.Az};
	float32_t power_sum;
	/*compute theta = atan(Ax/sqrt(Ay^2+Az^2))*/
	arm_power_f32(src,2,&power_sum);
	/*return with conversion from rad to deg*/
	return atan2(power_sum,accel.Ax)*(180/PI);
}

tAccelParameters calculateParameters(tAccel accel)
{
	tAccelParameters param;
	param.A_svm = calculate_A_svm(accel);
	param.Theta = calculate_Theta(accel);
	return param;
}

void accel_Config(void)
{
	accel_Write(0x2D,0x00); // set power_ctl to 0x00 disabling the module
	accel_Write(0x2D,0x08); // activate the module
	accel_Write(0X31,0x0B); // set data format full resolution
	accel_Write(0X2E	,0x00); // activate DATA_READY interrupt
	accel_Write(0X2F,0x80); // set DATA_READY interrupt to INT2 pin
	accel_Write(0X2E	,0x80); // activate DATA_READY interrupt
}

void accel_Write(unsigned char reg, unsigned char value)
{
	uint8_t data[2];
	data[0] = reg;
	data[1] = value;
	HAL_I2C_Master_Transmit(&hi2c2,DEVICE_ADRESS,data,2,100);	
}

tAccel filter_accel_data(tAccel accel)
{
	static tAccel filtered_accel = {0,0,0};
	if(filtered_accel.Ax == 0 && filtered_accel.Ay == 0 && filtered_accel.Az == 0)
	{
		filtered_accel = accel;
	}
	else
	{
		filtered_accel.Ax = (ALPHA_FILTER-1) * filtered_accel.Ax + ALPHA_FILTER*accel.Ax;
		filtered_accel.Ay = (ALPHA_FILTER-1) * filtered_accel.Ay + ALPHA_FILTER*accel.Ay;
		filtered_accel.Az = (ALPHA_FILTER-1) * filtered_accel.Az + ALPHA_FILTER*accel.Az;
	}
	return filtered_accel;
}

void accel_start_DMA()
{
	HAL_I2C_Mem_Read_DMA(&hi2c2,DEVICE_ADRESS,0x32,1,dma_buffer,6);
}

void accel_read_DMA(unsigned char* buffer)
{
		memcpy(buffer,dma_buffer,6);
}
