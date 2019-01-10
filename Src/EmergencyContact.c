#include "EmergencyContact.h"
#include "string.h"

#define FALSE 0
#define TRUE 0
typedef struct
{
	char m_contact[9];
	
}Contact_Typedef;



_Bool updateEmergencyContact(char*contact)
{
	if(sizeof(contact) != 9)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

char* getEmergencyContact()
{
	
}

