#ifndef __TEMPERATURE_H_
#define __TEMPERATURE_H_

#define SLAVE_ADRESS 0x5A << 1
#define OBJ_TEMP 0x07

#define TMAX 38.0f
#define TMIN 32.0f
#define MAX_TEMP 5

void TEMP_Init(void);
void resetTempCount(void);
int readTemperature(void);
void calculateTemperature(int);
_Bool checkTemperatureIssue(float lastTemperature);
float getLastTemperature(void);

#endif 
