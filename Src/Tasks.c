#include "stm32f7xx_hal.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "rtc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "string.h"
#include <stdbool.h>

#include "Temperature.h"
#include "Heartrate.h"
#include "Bluetooth.h"
#include "Buzzer.h"
#include "GSM.h"
#include "Accelerometer.h"
#include "ADXL345.h"
#include "Fall_Detection.h"
#include "Tasks.h"

/*Semaphores*/
SemaphoreHandle_t xSemaphoreBluetooth;
SemaphoreHandle_t xSemaphoreReadTemp;
SemaphoreHandle_t xSemaphoreSendBT;
SemaphoreHandle_t xSemaphoreStore;
SemaphoreHandle_t xSemaphoreDistress;
SemaphoreHandle_t xSemaphoreSoundAlert;
SemaphoreHandle_t xSemaphoreActivateDMA;
SemaphoreHandle_t xSemaphoreDMAFinish;

/*Mutexes*/
SemaphoreHandle_t xMutexTempStore;
SemaphoreHandle_t xMutexHRStore;
SemaphoreHandle_t xMutexSendBT;
SemaphoreHandle_t xMutexEC;

/*Queues*/
QueueHandle_t xQueueBT;
QueueHandle_t xQueueTempProcessed;
QueueHandle_t xQueueTempRaw;
QueueHandle_t xQueueHRRaw;
QueueHandle_t xQueueHRProcessed;
QueueHandle_t xQueueAccelProcessed;
QueueHandle_t xQueueAccelRaw;
QueueHandle_t xQueueAccelProcessed;

/*Queue set*/
QueueSetHandle_t xQueueSetAnalysis;

/* Struct to store data for app */
struct Storage{
	int hour;
	int minute;
	int BPM;
	float temp;
};

/* Data to be sent to bluetooth app */
char logData[4000];


void init_queue_set()
{
	// Init queue set
	xQueueSetAnalysis = xQueueCreateSet(1 * 3);
	// Add queues to set
	xQueueAddToSet(xQueueTempProcessed, xQueueSetAnalysis);
	xQueueAddToSet(xQueueHRProcessed, xQueueSetAnalysis);
	xQueueAddToSet(xQueueAccelProcessed, xQueueSetAnalysis);
}

void init_queues()
{
	xQueueBT = xQueueCreate(BUFFER_SIZE, sizeof(uint8_t));
	xQueueTempRaw = xQueueCreate(SINGLE_VALUE, sizeof(int));
	xQueueTempProcessed = xQueueCreate(SINGLE_VALUE, sizeof(float));
	xQueueHRRaw = xQueueCreate(HR_SAMPLES, sizeof(int));
	xQueueHRProcessed = xQueueCreate(SINGLE_VALUE, sizeof(int));
	xQueueAccelProcessed = xQueueCreate( 30 , sizeof(tAccelParameters) );
	xQueueAccelRaw = xQueueCreate( 30 , 6*sizeof(uint8_t) );
}

void init_mutexes()
{
	xMutexTempStore = xSemaphoreCreateMutex();
	xMutexHRStore = xSemaphoreCreateMutex();
	xMutexSendBT = xSemaphoreCreateMutex();
	xMutexEC = xSemaphoreCreateMutex();
}

void init_semaphores()
{
	xSemaphoreBluetooth = xSemaphoreCreateBinary();
	xSemaphoreReadTemp = xSemaphoreCreateBinary();
	xSemaphoreSendBT = xSemaphoreCreateBinary();
	xSemaphoreStore = xSemaphoreCreateBinary();
	xSemaphoreDistress = xSemaphoreCreateBinary();
	xSemaphoreActivateDMA = xSemaphoreCreateBinary();
	xSemaphoreDMAFinish = xSemaphoreCreateBinary();
	xSemaphoreSoundAlert = xSemaphoreCreateBinary();
}

void init_tasks()
{
	xTaskCreate((TaskFunction_t)vTaskProcessBT, "BT", 1024, NULL, 1, NULL);
	xTaskCreate((TaskFunction_t)vTaskSendBT, "SBT", 1024, NULL, 1, NULL);
	xTaskCreate((TaskFunction_t)vTaskProcessHeartRate, "HR", 2048, NULL, 2, NULL);
	xTaskCreate((TaskFunction_t)vTaskAcquireTemp, "ATEMP", 1024, NULL, 3, NULL);
	xTaskCreate((TaskFunction_t)vTaskProcessTemp, "PTEMP", 1024, NULL, 2, NULL);
	xTaskCreate((TaskFunction_t)vTaskStore, "STORE", 1024, NULL, 1, NULL);
	xTaskCreate((TaskFunction_t)vTaskAnalyze, "AN", 15000, NULL, 2, NULL);
	xTaskCreate((TaskFunction_t)vTaskProcessAccel, "ProcessAccel", 2048, NULL, 2, NULL);
	xTaskCreate((TaskFunction_t)vTaskAcquireAccel, "AcquireAccel", 1024, NULL, 3, NULL);
	xTaskCreate((TaskFunction_t)vTaskSoundAlert, "SoundAlert", 1024, NULL, 3, NULL);
	xTaskCreate((TaskFunction_t)vTaskDistress, "Distress", 1024, NULL, 3, NULL);
}

void vTaskProcessAccel()
{
	uint8_t accel_raw_data[6];
	tAccel accel;
	tAccelParameters accel_parameters;
	for(;;)
	{	
		/* read raw data from xQueueAccelRaw */
		xQueueReceive(xQueueAccelRaw,&accel_raw_data, portMAX_DELAY);
		/* convert raw data to g force*/
		accel = convertToGForce(accel_raw_data);
		/*low pass filter data*/
		accel = filter_accel_data(accel);
		/*calculate accelerometer parameters */
		accel_parameters = calculateParameters(accel);
		/* send parameters to task Analyze*/
		xQueueSend(xQueueAccelProcessed,(void*)&accel_parameters,( TickType_t ) 10);
	}
}

void vTaskAcquireAccel()
{
	uint8_t accel_raw_data[6];
	for(;;)
	{
		/* wait semaphore to read accelerometer sensor with DMA*/
		xSemaphoreTake(xSemaphoreActivateDMA,portMAX_DELAY);
		/*start DMA read*/
		accel_start_DMA();
		/*wait for DMA to finish */
		xSemaphoreTake(xSemaphoreDMAFinish,portMAX_DELAY);
		/*read data from DMA*/
		accel_read_DMA(accel_raw_data);
		/* send data to process data*/
		xQueueSend(xQueueAccelRaw,(void*)(&accel_raw_data),( TickType_t ) 10);
	}
}

void vTaskDistress()
{
	/* Start and configure GSM module */
	GSM_Config();
	/* Read contact saved on SIM card */
	readContact(&emergency_contact, 5);
	for(;;)
	{
		/*wait for distress event*/
		xSemaphoreTake(xSemaphoreDistress,portMAX_DELAY);
		/*activate sound alert task*/
		xSemaphoreGive(xSemaphoreSoundAlert);
		/*take mutex to read emergency contact*/
		xSemaphoreTake(xMutexEC,portMAX_DELAY);
		/*send sms distress message*/
		//sendSMS(&emergency_contact, "HELP");
		/*realease mutex*/
		xSemaphoreGive(xMutexEC);
	}
}

void vTaskSoundAlert()
{
	TickType_t xLastWakeTime;
	for(;;)
	{
		/*wait for sound alert*/
		xSemaphoreTake(xSemaphoreSoundAlert,portMAX_DELAY);
		/*get ticks*/
		xLastWakeTime = xTaskGetTickCount();
		/*start the buzzer*/
		startSoundAlert();
		/*delay the task to keep sounding*/
		vTaskDelayUntil(&xLastWakeTime,pdMS_TO_TICKS(3000));
		/*stop the sound alert*/
		stopSoundAlert();
	}
}

void vTaskAnalyze()
{
	/* 	
			From HRProcessed receives: int
			From TempProcessed receives: float
			From AccelProcessed receives: tAccelParameters
	*/
	int BPM;
	float temperature;
	tAccelParameters accelData;
	// Queue pointer
	QueueSetMemberHandle_t xQueueThatContainsData;

	for(;;)
	{
		xQueueThatContainsData = xQueueSelectFromSet(xQueueSetAnalysis, portMAX_DELAY);
		
		if(xQueueThatContainsData == NULL)
		{
			/* do nothing */
		}
		else if(xQueueThatContainsData == (QueueSetMemberHandle_t)xQueueTempProcessed)
		{
			/*temperature to be analyzed*/
			xQueueReceive(xQueueTempProcessed, &temperature, 0);
			/*check for fever or cold*/
			if(checkTemperatureIssue(temperature))
			{
				/*fever or cold detected*/
				xSemaphoreGive(xSemaphoreDistress);
			}		
		}
		else if(xQueueThatContainsData == (QueueSetMemberHandle_t)xQueueHRProcessed)
		{
			/*heart rate to be analyzed*/
			xQueueReceive(xQueueHRProcessed, &BPM, 0);
			/*check for tachycardia or bradycardia*/
			if(checkHeartRateIssue(BPM))
			{
				/*heart issue detected*/
				xSemaphoreGive(xSemaphoreDistress);
			}			
		}
		else if(xQueueThatContainsData == (QueueSetMemberHandle_t)xQueueAccelProcessed)
		{
			/*accelerometer data to be analysed*/
			xQueueReceive(xQueueAccelProcessed, &accelData, 0);
			/*check data for fall event*/
			if(fall_detection(accelData) == FALL)
			{
				/*fall event was detected*/
				xSemaphoreGive(xSemaphoreDistress);
			}
		}
	}
}

void vTaskProcessHeartRate()
{
	int queueData;
	int BPM;
	_Bool dataReady = false;
	for(;;)
	{
		// Receive new data from queue
		xQueueReceive(xQueueHRRaw, &queueData, portMAX_DELAY);
		// Check if ECG data buffer is full
		dataReady = storeECGData(queueData);
		// ECG data buffer is full
		if(dataReady)
		{
			// Apply filter to signal
			firFilter();
			// Iterate the data array in blocks of data
			if(searchBeat()){
				// Take BPM mutex
				xSemaphoreTake(xMutexHRStore, portMAX_DELAY);
				// Calculate BPM
				calculateHeartRate();
				// Get BPM Value
				BPM = getLastHeartRate();
				// Send last BPM value to queue
				xQueueSendToBack(xQueueHRProcessed, &BPM, portMAX_DELAY);
				// Give BPM mutex
				xSemaphoreGive(xMutexHRStore);
			}
		}
	}
}

void vTaskAcquireTemp()
{
	int temp;
	for(;;)
	{
		// Acquisition trigger
		xSemaphoreTake(xSemaphoreReadTemp, portMAX_DELAY);
		// Read sensor data
		temp = readTemperature();
		// Send raw data to processing task
		xQueueSendToBack(xQueueTempRaw, &temp, portMAX_DELAY);
	}
}

void vTaskProcessTemp()
{
	int temp;
	float newTemp;
	for(;;)
	{
		// Receive raw temperature data from queue
		xQueueReceive(xQueueTempRaw, &temp, portMAX_DELAY);
		// Take temperature mutex
		xSemaphoreTake(xMutexTempStore, portMAX_DELAY);
		// Calculate real temperature
		calculateTemperature(temp);
		// Get last temperature value
		newTemp = getLastTemperature();
		// Send for analysis
		xQueueSendToBack(xQueueTempProcessed, &newTemp, portMAX_DELAY);
		// Give the mutex
		xSemaphoreGive(xMutexTempStore);	
	}
}

void vTaskProcessBT()
{
	char queueData[30];
	char queueIndex;
	for(;;){
		xSemaphoreTake(xSemaphoreBluetooth, portMAX_DELAY);
		queueIndex = 0;
		while(uxQueueMessagesWaiting(xQueueBT) != 0){
			xQueueReceive(xQueueBT, &queueData[(queueIndex++)], 0);
		}
	parseCommand(queueData);
	executeCommand();
	}
}

void vTaskSendBT()
{
	for(;;)
	{
		// Take notification
		xSemaphoreTake(xSemaphoreSendBT, portMAX_DELAY);
		// Take mutex
		xSemaphoreTake(xMutexSendBT, portMAX_DELAY);
		// Send data via DMA to bluetooth module
		HAL_UART_Transmit_DMA(&huart6, (uint8_t*)logData, sizeof(logData));
		// Give mutex
		xSemaphoreGive(xMutexSendBT);	
	}	
}

void vTaskStore()
{
	RTC_TimeTypeDef* sTIME;
	struct Storage tStore;
	char messageToLog[20];
	
	for(;;)
	{
		// Trigger storage task
		xSemaphoreTake(xSemaphoreStore, portMAX_DELAY);
		// Get current time
		HAL_RTC_GetTime(&hrtc, sTIME, RTC_FORMAT_BIN);
		tStore.hour = sTIME->Hours;
		tStore.minute = sTIME->Minutes;
		sTIME->Hours+=1;
		HAL_RTC_SetTime(&hrtc, sTIME, RTC_FORMAT_BIN);
		// Get last temperature value
		xSemaphoreTake(xMutexTempStore, portMAX_DELAY);
		tStore.temp = getLastTemperature();
		xSemaphoreGive(xMutexTempStore);
		// Get last heart rate value
		xSemaphoreTake(xMutexHRStore, portMAX_DELAY);
		tStore.BPM = getLastHeartRate();
		xSemaphoreGive(xMutexHRStore);
		// Write on log buffer
		sprintf(messageToLog,"%d:%d %d %.1f \n", tStore.hour, tStore.minute, tStore.BPM, tStore.temp);
		xSemaphoreTake(xMutexSendBT, portMAX_DELAY);
		strcat(logData, messageToLog);
		xSemaphoreGive(xMutexSendBT);
	}
}
