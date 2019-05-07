#include "usart.h"
#include "dma.h"
#include "rtc.h"
#include "tim.h"
#include "Bluetooth.h"
#include "Tasks.h"
#include "GSM.h"
#include <stdlib.h>
#include <string.h>

static uint8_t ch = 0;

static t_cmd_struct cmd_table[] = {
	{"UT", updateTime},
	{"EC", changeEmergencyContact},
	{"LG", logRequest}
};

static t_str_array str_arr;

void BT_Init()
{
	HAL_UART_Receive_IT(&huart6, &ch, 1);
}

void Rx_Bt_Handler()
{
	static char delimiter = '\0';
	BaseType_t xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = pdFALSE;

	switch(ch)
	{
		// Check for carriage return
		case(CR):
			// Send carriage return for parsin purpose
			xQueueSendToBackFromISR(xQueueBT, &delimiter, &xHigherPriorityTaskWoken);
		// Signal vTaskProcessBT task
			xSemaphoreGiveFromISR( xSemaphoreBluetooth, &xHigherPriorityTaskWoken );
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
			break;
		default:
			// Add new character to queue
			xQueueSendToBackFromISR(xQueueBT, &ch, &xHigherPriorityTaskWoken);
		break;
	}
	// Enable interrupts
	HAL_UART_Receive_IT(&huart6, &ch, 1);	
}

void parseCommand(char * queueData)
{
	char index = 0;
	char *token = " ";
	token = strtok(queueData, " .");
	while(token != NULL)
	{
		strcpy(str_arr.stringArray[index],token);
		index = index + 1;
		token = strtok(NULL," ");	
	}
	str_arr.size = index;
}

void executeCommand()
{
	size_t index;
	// Search for the received command in the command array
	for(index = 0; index < NCOMMANDS; index++)
	{
		// Compare command identifier: LG, UT or EC
		if(strcmp(cmd_table[index].command, str_arr.stringArray[COMMAND_INDEX]) == 0)
			// Command found, execute the corresponding function
			cmd_table[index].command_function();
	}
}

void updateTime()
{	
	RTC_TimeTypeDef * sTIME;
	// Save received hour data
	sTIME->Hours = atoi(str_arr.stringArray[HOUR]);
	sTIME->Minutes = atoi(str_arr.stringArray[MINUTE]);
	// Check received time
	if(sTIME->Hours <= 23 && sTIME->Minutes <= 59)
	{
		// set RTC to current time
		HAL_RTC_SetTime(&hrtc, sTIME, RTC_FORMAT_BIN);
		// send feedback message
		HAL_UART_Transmit_DMA(&huart6, (uint8_t*)"OK", 2);
		// Start timer 2 interrupts
		HAL_TIM_Base_Start_IT(&htim2);
	} else 
	// Error
	HAL_UART_Transmit_DMA(&huart6, (uint8_t*)"NO", 2);
}

void changeEmergencyContact()
{
	Contact_Typedef contact;
	int phoneNumber;
	phoneNumber = atoi(str_arr.stringArray[CONTACT]);
	if(phoneNumber > 900000000 && phoneNumber < 999999999)
	{
		// Add the number indicative
		strcat(contact.m_number, "+351");
		// Concatenate the phone number with the indicative
		strcat(contact.m_number, str_arr.stringArray[CONTACT]);
		// take mutex
		xSemaphoreTake(xMutexEC, portMAX_DELAY);
		// Update global variable
		emergency_contact = contact;
		// update emergency contact function
		if(addContact(contact, 5))
			// Number was saved
			HAL_UART_Transmit_DMA(&huart6, (uint8_t*)"OK", 2);
		else 
			// Number is invalud
			HAL_UART_Transmit_DMA(&huart6, (uint8_t*)"NO", 2);
		// give mutex
		xSemaphoreGive(xMutexEC);
	}
	else 
		HAL_UART_Transmit_DMA(&huart6, (uint8_t*)"NO", 2);
}

void logRequest()
{
	// Signal the vTaskSendBT task
	xSemaphoreGive(xSemaphoreSendBT);
}

