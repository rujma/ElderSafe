#ifndef __BLUETOOTH_H_
#define __BLUETOOTH_H_

#define FALSE 0
#define TRUE 1
#define CR 0x0D
#define NCOMMANDS 3
#define BUFFER_SIZE 32
#define COMMAND_INDEX 0
#define HOUR 1
#define MINUTE 2
#define CONTACT 1

// Received commands
typedef struct{
	char stringArray[5][30];
	unsigned char size;
}t_str_array;

// Table of commands
typedef struct{
	char command[2];
	void (*command_function)(void);
}t_cmd_struct;

// Table of commands function
void updateTime(void);
void changeEmergencyContact(void);
void logRequest(void);


// Initializations
void BT_Init(void);
void Rx_Bt_Handler(void);

// Support functions
void parseCommand(char*);
void executeCommand(void);

#endif
