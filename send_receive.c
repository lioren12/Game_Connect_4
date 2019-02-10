// send_receive.c

/*
Description - implementation of the send/receive threads and its related functions.
*/

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

// Project Includes --------------------------------------------------------------------

#include "send_receive.h"

// Declarations --------------------------------------------------------------------

/*
Parameters - none.
Returns - none.
Description - waits until signaled that a new data is ready for sending to server.
*/
void WaitForSendToServerSemaphore();

/*
Parameters - none.
Returns - none.
Description - creates and sends new user request message to server.
*/
void HandleNewUserRequest();

/*
Parameters - ReceivedDataFromServer: the data that was received from server.
Returns - none.
Description - checks the request type received from the server and handles it accordingly.
*/
void HandleReceivedDataFromServer(char *ReceivedDataFromServer);

/*
Parameters - ReceivedDataFromServer: the data that was received from server.
Returns - none.
Description - parses and updates client database accordingly. also prints as instructed to client screen.
*/
void HandleNewUserAccept(char *ReceivedDataFromServer);

/*
Parameters - ReceivedDataFromServer: the data that was received from server.
Returns - none.
Description - prints as instructed the game board to client screen.
*/
void HandleBoardView(char *ReceivedDataFromServer);

/*
Parameters - ReceivedDataFromServer: the data that was received from server.
Returns - none.
Description - prints as instructed to client screen.
*/
void HandleTurn(char *ReceivedDataFromServer);

/*
Parameters - ReceivedDataFromServer: the data that was received from server.
Returns - none.
Description - prints as instructed the error message to client screen.
*/
void HandlePlayDeclined(char *ReceivedDataFromServer);

/*
Parameters - ReceivedDataFromServer: the data that was received from server.
Returns - none.
Description - handle game ended message and prints as instructed to client screen.
*/
void HandleGameEnded(char *ReceivedDataFromServer);

/*
Parameters - ReceivedDataFromServer: the data that was received from server.
Returns - none.
Description - prints as instructed to client screen.
*/
void HandleReceiveMessage(char *ReceivedDataFromServer);

// Function Definitions --------------------------------------------------------------------

void WINAPI SendThread()
{
	int SendDataToServerReturnValue;
	HandleNewUserRequest();
	while (TRUE) {
		WaitForSendToServerSemaphore();
		if (STRINGS_ARE_EQUAL(Client.MessageToSendToServer, "FINISHED")) {
			break; // finished communication
		}
		SendDataToServerReturnValue = SendData(Client.Socket, Client.MessageToSendToServer, Client.LogFilePtr);
		if (SendDataToServerReturnValue == ERROR_CODE) {
			WriteToLogFile(Server.LogFilePtr, "Custom message: failed with TRNS_FAILED.\n");
			CloseSocketThreadsAndWSAData();
			exit(ERROR_CODE);
		}
		char TempMessage[MESSAGE_LENGTH];
		sprintf(TempMessage, "Sent to server: %s\n", Client.MessageToSendToServer);
		WriteToLogFile(Client.LogFilePtr, TempMessage);
	}
}


void HandleNewUserRequest()
{
	char NewUserRequest[MESSAGE_LENGTH];
	int SendDataToServerReturnValue;
	sprintf(NewUserRequest, "NEW_USER_REQUEST:%s\n", Client.UserName);
	SendDataToServerReturnValue = SendData(Client.Socket, NewUserRequest, Client.LogFilePtr);
	if (SendDataToServerReturnValue == ERROR_CODE) {
		WriteToLogFile(Server.LogFilePtr, "Custom message: failed with TRNS_FAILED.\n");
		CloseSocketThreadsAndWSAData();
		exit(ERROR_CODE);
	}
	char TempMessage[MESSAGE_LENGTH];
	sprintf(TempMessage, "Sent to server: %s\n", NewUserRequest);
	WriteToLogFile(Client.LogFilePtr, TempMessage);
}


void WaitForSendToServerSemaphore()
{
	DWORD wait_code;

	wait_code = WaitForSingleObject(Client.SendToServerSemaphore, INFINITE); // wait for send to server notification
	if (WAIT_OBJECT_0 != wait_code) {
		WriteToLogFile(Client.LogFilePtr, "Custom message: SendThread - failed to wait for  UserInterface semaphore.\n");
		CloseSocketThreadsAndWSAData();
		exit(ERROR_CODE);
	}
}


void WINAPI ReceiveThread()
{
	char *ReceivedData = NULL;
	while (TRUE) {
		if (!Client.GotExitFromUserOrGameFinished) 
		{
			ReceivedData = ReceiveData(Client.Socket, Client.LogFilePtr);
			if (ReceivedData == NULL) {
				WriteToLogFile(Server.LogFilePtr, "Custom message: failed with TRNS_FAILED.\n");
				CloseSocketThreadsAndWSAData();
				exit(ERROR_CODE);
			}
		}
		if (Client.GotExitFromUserOrGameFinished || STRINGS_ARE_EQUAL(ReceivedData, "FINISHED")) {  
			if (Client.GotExitFromUserOrGameFinished) {
				strcpy(Client.MessageToSendToServer, "FINISHED");
				if (ReleaseOneSemaphore(Client.SendToServerSemaphore) == FALSE) { // signal sending thread to finish
					WriteToLogFile(Client.LogFilePtr, "Custom message: ReceiveThread - failed to release SendToServer semaphore.\n");
					CloseSocketThreadsAndWSAData();
					exit(ERROR_CODE);
				}
				if (Client.ThreadHandles[2] != NULL) { // if user interface is active
					if (TerminateThread(Client.ThreadHandles[2], WAIT_OBJECT_0) == FALSE) {
						WriteToLogFile(Client.LogFilePtr, "Custom message: ReceiveThread - failed to terminate user interface thread.\n");
						CloseSocketThreadsAndWSAData();
						exit(ERROR_CODE);
					}
				}
				break;
			}
			else { // server disconnected - need to exit now
				OutputMessageToWindowAndLogFile(Client.LogFilePtr, "Server disconnected. Exiting.\n");
				CloseSocketThreadsAndWSAData();
				exit(ERROR_CODE);
			}
		}
		HandleReceivedDataFromServer(ReceivedData);
	}
}


void HandleReceivedDataFromServer(char *ReceivedDataFromServer) {
	char TempMessage[MESSAGE_LENGTH];
	if (FIRST_N_ON_STRINGS_ARE_EQUAL(ReceivedDataFromServer, "NEW_USER_ACCEPTED", NEW_USER_ACCEPTED_OR_DECLINED_SIZE) ||
		FIRST_N_ON_STRINGS_ARE_EQUAL(ReceivedDataFromServer, "NEW_USER_DECLINED", NEW_USER_ACCEPTED_OR_DECLINED_SIZE)) {
		HandleNewUserAccept(ReceivedDataFromServer);
	}
	else if (FIRST_N_ON_STRINGS_ARE_EQUAL(ReceivedDataFromServer, "GAME_STARTED", GAME_STARTED_SIZE)) {
		printf("Game is on!\n");
		IsGameStarted = TRUE;
	}
	else if (FIRST_N_ON_STRINGS_ARE_EQUAL(ReceivedDataFromServer, "BOARD_VIEW:", BOARD_VIEW_SIZE)) {
		HandleBoardView(ReceivedDataFromServer);
	}
	else if (FIRST_N_ON_STRINGS_ARE_EQUAL(ReceivedDataFromServer, "TURN_SWITCH:", TURN_SWITCH_SIZE)) {
		HandleTurn(ReceivedDataFromServer);
	}
	else if (FIRST_N_ON_STRINGS_ARE_EQUAL(ReceivedDataFromServer, "RECEIVE_MESSAGE:", RECEIVE_MESSAGE_SIZE)) {
		HandleReceiveMessage(ReceivedDataFromServer);
	}
	else if (FIRST_N_ON_STRINGS_ARE_EQUAL(ReceivedDataFromServer, "PLAY_ACCEPTED", PLAY_ACCEPTED_SIZE)) {
		printf("Well played\n");
	}
	else if (FIRST_N_ON_STRINGS_ARE_EQUAL(ReceivedDataFromServer, "PLAY_DECLINED:", PLAY_DECLINED_SIZE)) {
		HandlePlayDeclined(ReceivedDataFromServer);
	}
	else if (FIRST_N_ON_STRINGS_ARE_EQUAL(ReceivedDataFromServer, "GAME_ENDED:", GAME_ENDED_SIZE)) {
		HandleGameEnded(ReceivedDataFromServer);
	}

	else {
		sprintf(TempMessage, "Custom message: Got unexpected answer from Server:\n%s.\nExiting...\n", ReceivedDataFromServer);
		WriteToLogFile(Client.LogFilePtr, TempMessage);
		free(ReceivedDataFromServer);
		CloseSocketThreadsAndWSAData();
		exit(ERROR_CODE);
	}
	sprintf(TempMessage, "Received from server: %s\n", ReceivedDataFromServer);
	WriteToLogFile(Client.LogFilePtr, TempMessage);
	free(ReceivedDataFromServer);
}


void HandleNewUserAccept(char *ReceivedDataFromServer)
{
	if (FIRST_N_ON_STRINGS_ARE_EQUAL(ReceivedDataFromServer, "NEW_USER_DECLINED", NEW_USER_ACCEPTED_OR_DECLINED_SIZE)) {
		char TempMessage[MESSAGE_LENGTH]; 
		sprintf(TempMessage, "Received from server: %s\n", ReceivedDataFromServer); 
		WriteToLogFile(Client.LogFilePtr, TempMessage); 
		printf("Request to join was refused.\n");
		free(ReceivedDataFromServer);
		CloseSocketThreadsAndWSAData();
		exit(ERROR_CODE);
	}
	if (FIRST_N_ON_STRINGS_ARE_EQUAL(ReceivedDataFromServer + NEW_USER_ACCEPTED_OR_DECLINED_SIZE + 1, "1", 1)) {
		Client.PlayerType = R;
	}
	else if (FIRST_N_ON_STRINGS_ARE_EQUAL(ReceivedDataFromServer + NEW_USER_ACCEPTED_OR_DECLINED_SIZE + 1, "2", 1)) {
		Client.PlayerType = Y;
	}
	else {
		WriteToLogFile(Client.LogFilePtr, "Custom message: Got unexpected answer from Server. Exiting...\n");
		free(ReceivedDataFromServer);
		CloseSocketThreadsAndWSAData();
		exit(ERROR_CODE);
	}
	if (ReleaseOneSemaphore(Client.UserInterfaceSemaphore) == FALSE) { // to release UserInterface thread
		WriteToLogFile(Client.LogFilePtr, "Custom message: SendThread - failed to release receive semaphore.\n");
		CloseSocketThreadsAndWSAData();
		exit(ERROR_CODE);
	}
}


void HandleBoardView(char *ReceivedDataFromServer)
{ 
	Client.Is_hConsole = TRUE;
	int row, column;

	int Index = BOARD_VIEW_LINE_OFFSET - 1;
	for (row = 0; row < BOARD_HEIGHT; row++)
	{
		for (column = 0; column < BOARD_WIDTH; column++)
		{
			printf("| ");
			Index++;
			if (FIRST_N_ON_STRINGS_ARE_EQUAL(ReceivedDataFromServer + Index, "R", 1))
				SetConsoleTextAttribute(Client.hConsole, RED);
			if (FIRST_N_ON_STRINGS_ARE_EQUAL(ReceivedDataFromServer + Index, "Y", 1))
				SetConsoleTextAttribute(Client.hConsole, YELLOW);

			printf("O");

			SetConsoleTextAttribute(Client.hConsole, BLACK);
			printf(" ");
		}
		//Draw dividing line between the rows
		printf("\n");
		for (column = 0; column < BOARD_WIDTH; column++)
		{
			printf("----");
		}
		printf("\n");
		Index++;
	}
}


void HandleReceiveMessage(char *ReceivedDataFromServer)
{
	char MessageToLog[MESSAGE_LENGTH] = "";
	char MessageToPrint[MESSAGE_LENGTH] = "";
	char FirstParameter[USER_NAME_LENGTH] = "";
	int MessageStartPosition = RECEIVE_MESSAGE_SIZE;

	// Cut the user name (the first param)
	int MessageEndPosition = MessageStartPosition;
	while (ReceivedDataFromServer[MessageEndPosition] != ';') {
		MessageEndPosition++;
	}
	int UserNameLength = MessageEndPosition - MessageStartPosition;
	strncpy(FirstParameter, ReceivedDataFromServer + MessageStartPosition, UserNameLength);

	// Cut the rest of the message without ";" - to print
	strncpy(MessageToPrint, FirstParameter, UserNameLength);
	strcat(MessageToPrint, ": ");
	MessageEndPosition = MessageStartPosition + UserNameLength + 1;
	int MessagePrevPosition;

	while (ReceivedDataFromServer[MessageEndPosition] != '\0')
	{
		MessagePrevPosition = MessageEndPosition;
		while (ReceivedDataFromServer[MessageEndPosition] != ';')
		{
			if (ReceivedDataFromServer[MessageEndPosition] == '\n')
				break;
			MessageEndPosition++;
		}
		strncat(MessageToPrint, ReceivedDataFromServer + MessagePrevPosition, MessageEndPosition - MessagePrevPosition);
		MessageEndPosition++;
	}
	printf("%s\n", MessageToPrint);
}


void HandleTurn(char *ReceivedDataFromServer) {
	char TurnMessage[MESSAGE_LENGTH];
	char PlayerName[USER_NAME_LENGTH];
	int MessageStartPosition = TURN_SWITCH_SIZE;

	int MessageEndPosition = MessageStartPosition;
	while (ReceivedDataFromServer[MessageEndPosition] != '\n') { 
		MessageEndPosition++;
	}
	int MessageLength = MessageEndPosition - MessageStartPosition;
	strncpy(PlayerName, ReceivedDataFromServer + MessageStartPosition, MessageLength);
	PlayerName[MessageLength] = '\0';
	sprintf(TurnMessage, "%s's turn\n", PlayerName); 
	printf("%s",TurnMessage);

	// For file mode: update who's current turn
	if (STRINGS_ARE_EQUAL(PlayerName, Client.UserName))
		WhosTurn = Client.PlayerType;
	else
	{
		if (Client.PlayerType == R) WhosTurn = Y;
		if (Client.PlayerType == Y) WhosTurn = R;
	}
}


void HandlePlayDeclined(char *ReceivedDataFromServer) {
	int EndOfMessageOffset = FindEndOfDataSymbol(ReceivedDataFromServer);
	char PlayerDeclinedMessage[MESSAGE_LENGTH];
	strcpy(PlayerDeclinedMessage, "Error: ");
	strncat(PlayerDeclinedMessage, ReceivedDataFromServer + PLAY_DECLINED_SIZE, (EndOfMessageOffset - PLAY_DECLINED_SIZE));
	strcat(PlayerDeclinedMessage, "\n");
	OutputMessageToWindowAndLogFile(Client.LogFilePtr, PlayerDeclinedMessage);
}


void HandleGameEnded(char *ReceivedDataFromServer) {
	if (FIRST_N_ON_STRINGS_ARE_EQUAL(ReceivedDataFromServer + GAME_ENDED_SIZE, "Tie", 3)) {
		printf("Game ended. Everybody wins!\n");
	}
	else {
		char GameEndedMessage[MESSAGE_LENGTH];
		char UserName[USER_NAME_LENGTH];
		int ParameterEndPosition = FindEndOfDataSymbol(ReceivedDataFromServer);
		strncpy(UserName, ReceivedDataFromServer + GAME_ENDED_SIZE, ParameterEndPosition - GAME_ENDED_SIZE);
		UserName[ParameterEndPosition - GAME_ENDED_SIZE] = '\0';
		sprintf(GameEndedMessage, "Game ended. The winner is %s!\n", UserName);
		printf("%s",GameEndedMessage); 
	}
	Client.GotExitFromUserOrGameFinished = true;
	shutdown(Client.Socket, SD_BOTH);
}
