#ifndef __TASKS_H_
#define __TASKS_H_
#include "cmsis_os.h"

#define SINGLE_VALUE 1

/*Semaphores*/
extern SemaphoreHandle_t xSemaphoreBluetooth;
extern SemaphoreHandle_t xSemaphoreReadTemp;
extern SemaphoreHandle_t xSemaphoreSendBT;
extern SemaphoreHandle_t xSemaphoreStore;
extern SemaphoreHandle_t xSemaphoreDistress;
extern SemaphoreHandle_t xSemaphoreSoundAlert;
extern SemaphoreHandle_t xSemaphoreActivateDMA;
extern SemaphoreHandle_t xSemaphoreDMAFinish;

/*Mutexes*/
extern SemaphoreHandle_t xMutexTempStore;
extern SemaphoreHandle_t xMutexHRStore;
extern SemaphoreHandle_t xMutexSendBT;
extern SemaphoreHandle_t xMutexEC;

/*Queues*/
extern QueueHandle_t xQueueBT;
extern QueueHandle_t xQueueTempProcessed;
extern QueueHandle_t xQueueTempRaw;
extern QueueHandle_t xQueueHRRaw;
extern QueueHandle_t xQueueHRProcessed;
extern QueueHandle_t xQueueAccelProcessed;
extern QueueHandle_t xQueueAccelRaw;
extern QueueHandle_t xQueueAccelProcessed;

/*Queue Set*/
extern QueueSetHandle_t xQueueSetAnalysis;

/*Initialization functions*/
void init_semaphores(void);
void init_queue_set(void);
void init_queues(void);
void init_mutexes(void);
void init_tasks(void);

/*Tasks*/
void vTaskProcessAccel(void);
void vTaskAcquireAccel(void);
void vTaskDistress(void);
void vTaskSoundAlert(void);
void vTaskAnalyze(void);
void vTaskProcessHeartRate(void);
void vTaskAcquireTemp(void);
void vTaskProcessTemp(void);
void vTaskProcessBT(void);
void vTaskSendBT(void);
void vTaskStore(void);

#endif
