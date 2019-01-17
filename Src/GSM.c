#include "string.h"
#include "usart.h"
#include "GSM.h"
#define CTRL_Z "\x1A"
#define CR "\r\n"
#define FALSE 0
#define TRUE 1



static GSM_t gsm;

void GSM_Init(void)
{
	MX_UART5_Init();
}
void GSM_Config(void)
{
	// Send wake up AT command
	GSM_send_message_check_answer("AT\r\n","AT\r\r\nOK\r\n");
	// Configure module for SMS mode
	GSM_send_message_check_answer("AT+CMGF = 1\r\n","AT+CMGF = 1\r\r\nOK\r\n");
}

_Bool addContact(Contact_Typedef contact, uint8_t pos)
{
	char msg[50];
	char answer[100];
	sprintf(msg, "AT+CPBW =%d,\"", pos);
	strcat(msg,contact.m_number);
	strcat(msg, "\",129,\"Emergency\"");
	strcpy(answer, msg);
	strcat(answer, "\r\r\nOK\r\n");
	strcat(msg,CR);
	return GSM_send_message_check_answer(msg, answer); 	
}
_Bool readContact(Contact_Typedef *contact, uint8_t pos)
{
	char response[50];
	char msg[20];
	char* ptr_token;
	sprintf(msg,"AT+CPBR=%d\r\n",pos); /* send AT+CPBR to read contact from entrie book in position pos*/
	GSM_send_message(msg); /* send msg to uart interface*/
	HAL_UART_Receive(&huart5,(uint8_t*)response,50,200); /* receive response from GSM module +CPBR:<index1>,<number>,<type>,<text><CR><LF>*/
	ptr_token = strtok(response,"\""); /* search for number in response*/
	ptr_token = strtok(NULL,"\"");
	if(ptr_token != NULL && strcmp(ptr_token,"") != 1) /* if number found store it in contact variable*/
	{
		strcpy(contact->m_number,ptr_token);
		return TRUE;
	}
	return FALSE;
}

_Bool sendSMS(Contact_Typedef *contact,char* message)
{
	char at_cmd[100];
	char resposta[100];
	strcpy(at_cmd,"AT+CMGS=\"");
	strcat(at_cmd,contact->m_number);
	strcat(at_cmd,"\"\r");
	GSM_send_message(at_cmd);/*send AT+CMGS="+xxxXXXXXXXXX" to send msg*/ 
	HAL_UART_Receive(&huart5,(uint8_t*)resposta,strlen(at_cmd)+4,300);
	GSM_send_message(message);/* send sms message*/
	GSM_send_message(CTRL_Z); /*send ctrl_z*/
	HAL_UART_Receive(&huart5,(uint8_t*)resposta,15,300);
	return TRUE;
}

_Bool GSM_send_message(char*message)
{
	if(strlen(message) != 0)
	{
		/*send message*/
		if(HAL_UART_Transmit(&huart5, (uint8_t*)message, strlen(message),200)==HAL_OK)
			return TRUE;
	}
	return FALSE;
}
_Bool GSM_send_message_check_answer(char*message,char* answer)
{
	char response[100];
	if(!gsm.busy && strlen(message) != 0)
	{
		/*send message*/
		if(HAL_UART_Transmit(&huart5, (uint8_t*)message, strlen(message),200)==HAL_OK)
		{
			HAL_UART_Receive(&huart5,(uint8_t*)response,strlen(answer),500);
			if(strncmp(answer,response,strlen(answer)) == 0)
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

