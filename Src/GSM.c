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
	GSM_send_message_check_answer("AT\r\n","AT\r\r\nOK\r\n");
	GSM_send_message_check_answer("AT+CMGF = 1\r\n","AT+CMGF = 1\r\r\nOK\r\n");
}

_Bool addContact(Contact_Typedef contact, uint8_t pos)
{
	char msg[50];
	if( strlen(contact.m_number) == 13 )
	{
		strcpy(msg,"AT+CPBW =1,");
		strcat(msg,contact.m_number);
		strcat(msg,",");
		strcat(msg,CR);
		GSM_send_message(msg);
		if(HAL_UART_Transmit(&huart5, (uint8_t*)msg, sizeof(msg), HAL_MAX_DELAY) == HAL_OK)
				return TRUE;
	}
		return FALSE;
}
_Bool readContact(Contact_Typedef *contact, uint8_t pos)
{
	char resposta[100];
	char msg[20];
	char* ptr_token;
	sprintf(msg,"AT+CPBR=%d\r\n",pos); /* send AT+CPBR to read contact from entrie book in position pos*/
	GSM_send_message(msg); /* send msg to uart interface*/
	HAL_UART_Receive(&huart5,(uint8_t*)resposta,50,100); /* receive response from GSM module +CPBR:<index1>,<number>,<type>,<text><CR><LF>*/
	ptr_token = strtok(resposta,"1"); /* search for number in response*/
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
	HAL_UART_Receive(&huart5,(uint8_t*)resposta,strlen(at_cmd)+4,10000);
	GSM_send_message(message);/* send sms message*/
	HAL_UART_Receive(&huart5,(uint8_t*)resposta,strlen(message),10000);
	GSM_send_message(CTRL_Z); /*send ctrl_z*/
	HAL_UART_Receive(&huart5,(uint8_t*)resposta,15,1000);
	return TRUE;
}

_Bool GSM_send_message(char*message)
{
	if(strlen(message) != 0)
	{
		/*send message*/
		if(HAL_UART_Transmit(&huart5, (uint8_t*)message, strlen(message),100)==HAL_OK)
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
		if(HAL_UART_Transmit(&huart5, (uint8_t*)message, strlen(message),100)==HAL_OK)
		{
			HAL_UART_Receive(&huart5,(uint8_t*)response,strlen(answer),1000);
			if(strncmp(answer,response,strlen(answer)) == 0)
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

