// user_interface.c

/*
Description - implementation of the user interface thread and its related functions.
*/

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS


// Project Includes --------------------------------------------------------------------

#include "user_interface.h"

// Declarations --------------------------------------------------------------------

/*
Parameters - none.
Returns - none.
Description - waits until user is accepted and user interface is opened to user.
*/
void WaitForUserInterfaceSemaphore();

/*
Parameters - none.
Returns - none.
Description - handles input from user and signals to sending thread when client makes a request.
*/
void HandleInputFromUser(char *UserInput);

/*
Parameters - UserInput: data from user
Returns - zero to success. 
		(-1) for flag => enter right after message
Description - handles the case when send message is sent by user
*/
int HandleSendMessage(char *UserInput);

// Function Definitions --------------------------------------------------------------------

void WINAPI UserInterfaceThread() {

	WaitForUserInterfaceSemaphore();
	char UserInput[USER_INPUT_LENGTH];
	while (TRUE) {
		if (STRINGS_ARE_EQUAL(Client.MessageToSendToServer, "FINISHED")) {
			break; // finished communication
		}
		if (STRINGS_ARE_EQUAL(Client.InputMode, "human")) {
			scanf("%s", UserInput);
			HandleInputFromUser(UserInput);
		}
		else if (IsGameStarted) {
			Sleep(SEND_MESSAGES_FILE_WAIT);
			if (Client.PlayerType == WhosTurn)
			{
				fscanf(Client.InputFilePtr, "%s", UserInput);
				HandleInputFromUser(UserInput);
			}
		}
	}
}

void WaitForUserInterfaceSemaphore() {
	DWORD wait_code;

	wait_code = WaitForSingleObject(Client.UserInterfaceSemaphore, INFINITE); // wait for connection to be established and user accepted
	if (WAIT_OBJECT_0 != wait_code) {
		WriteToLogFile(Client.LogFilePtr, "Custom message: SendThread - failed to wait for UserInterface semaphore.\n");
		CloseSocketThreadsAndWSAData();
		exit(ERROR_CODE);
	}
}

void HandleInputFromUser(char *UserInput)  
{
	bool NeedToReleaseSendToServerSemaphore = true;
	if (STRINGS_ARE_EQUAL(UserInput, "play")) {  
		if (STRINGS_ARE_EQUAL(Client.InputMode, "human")) {
			scanf("%s", UserInput); // get column
		}
		else if (IsGameStarted) {//file
				Sleep(SEND_MESSAGES_FILE_WAIT);
				fscanf(Client.InputFilePtr, "%s", UserInput);
			}
		if (strlen(UserInput) > 1)
		{
			printf("Error: Illegal move\n"); 
			NeedToReleaseSendToServerSemaphore = false; 
		}
		else {
			int ColumnNum = atoi(UserInput);
			sprintf(Client.MessageToSendToServer, "PLAY_REQUEST:%d\n", ColumnNum);
		}
	}
	else if (STRINGS_ARE_EQUAL(UserInput, "exit")) {
		printf("User is out, game is ended.");
		strcpy(Client.MessageToSendToServer, "FINISHED");
		Client.GotExitFromUserOrGameFinished = true;
		shutdown(Client.Socket, SD_BOTH);
		NeedToReleaseSendToServerSemaphore = false;
	}
	else if (STRINGS_ARE_EQUAL(UserInput, "message")) 
	{
		if (HandleSendMessage(UserInput) == (-1))
		{
			printf("Error: Illegal command\n");
			NeedToReleaseSendToServerSemaphore = false;
		}
	}

	else {
		printf( "Error: Illegal command\n"); 
		NeedToReleaseSendToServerSemaphore = false;
	}
	if (NeedToReleaseSendToServerSemaphore) {
		if (ReleaseOneSemaphore(Client.SendToServerSemaphore) == FALSE) { // signal sending thread that a message is ready for sending
			WriteToLogFile(Client.LogFilePtr, "Custom message: SendThread - failed to release SendToServer semaphore.\n");
			CloseSocketThreadsAndWSAData();
			exit(ERROR_CODE);
		}
	}
}

int HandleSendMessage(char *UserInput)
{
	char UserInputMessage[MESSAGE_LENGTH] = "";
	char ch;

	if (STRINGS_ARE_EQUAL(Client.InputMode, "human")) {
		gets(UserInput);
	}
	else if (IsGameStarted){
		Sleep(SEND_MESSAGES_FILE_WAIT);
		fgets(UserInput, MESSAGE_LENGTH, Client.InputFilePtr);
	}

	// check for enter right after message
	if (STRINGS_ARE_EQUAL(UserInput, "\0")) 
	{
		return (ERROR_CODE);
	}

	ch = UserInput[0];
	int CurrentIndex = 0;
	int StartParmIndex = 1;
	while (ch != "\n")
	{
		CurrentIndex++;
		ch = UserInput[CurrentIndex];
		if (ch == '\0')
		{
			strncat(UserInputMessage, UserInput + StartParmIndex, CurrentIndex - StartParmIndex);
			break;
		}
		if (ch == ' ')
		{
			strncat(UserInputMessage, UserInput + StartParmIndex, CurrentIndex - StartParmIndex);
			strcat(UserInputMessage, "; ;");
			StartParmIndex = CurrentIndex + 1;
		}
	}
	sprintf(Client.MessageToSendToServer, "SEND_MESSAGE:%s\n", UserInputMessage);
	return SUCCESS_CODE;
}
