#ifndef __GSM_H_
#define __GSM_H_
typedef enum
{
	GSM_OK,
	GSM_ERROR
}Gps_State_Typedef;

typedef struct
{
	char m_number[13];
}Contact_Typedef;

typedef struct
{
	unsigned char busy;
}GSM_t;

void GSM_Config(void);
_Bool addContact(Contact_Typedef contact, unsigned char pos);
_Bool sendSMS(Contact_Typedef *contact,char* message);
_Bool GSM_send_message(char*message);
_Bool GSM_send_message_check_answer(char*message,char* answer);
_Bool readContact(Contact_Typedef *contact, unsigned char pos);

#endif

