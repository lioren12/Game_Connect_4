// game_connect_four.c

/*
Description - implementation of all in-game related operations of the server - game management and clients handling.
*/

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

// Project Includes --------------------------------------------------------------------

#include "game_connect_four.h"
#include "handle_server.h"

// Declarations --------------------------------------------------------------------

/*
Parameters - ClientIndex: the index of the client.
Returns - true if the new user request was accepted and false if declined.
Description - check if new user is accepted and send reply to client.
*/
bool HandleNewUserRequestAndAccept(int ClientIndex);

/*
Parameters - ClientIndex: the index of the client, ReceivedDataFromClient: the request that was received from client.
Returns - none.
Description - parse new user request sent from client.
*/
void ParseNewUserRequest(char *ReceivedDataFromClient, int ClientIndex);

/*
Parameters - none.
Returns - none.
Description - when new user is accepted update the number of connected users.
			  if two users are connected signal that game has started.
*/
void UpdateNumberOfConnectedUsers();

/*
Parameters - ClientIndex: the index of the client.
Returns - none.
Description - when game starts - send game started, board view, and turn switch messages to the client.
*/
void SendGameStartedBoardViewAndTurnSwitch(int ClientIndex);

/*
Parameters - ClientIndex: the index of the client.
Returns - none.
Description - sends to client board view.
*/
void SendBoardView(int ClientIndex);

/*
Parameters - ClientIndex: the index of the client.
Returns - none.
Description - sends to client turn switch.
*/
void SendTurn(int ClientIndex);

/*
Parameters - ClientIndex: the index of the client, ReceivedDataFromClient: the data that was received from client.
Returns - none.
Description - checks the request type received from the client and handles it accordingly.
*/
void HandleReceivedDataFromClient(char *ReceivedDataFromClient, int ClientIndex);

/*
Parameters - ClientIndex: the index of the client, ReceivedDataFromClient: the data that was received from client.
Returns - none.
Description - handles play request from client.
*/
void HandlePlayRequest(int ClientIndex, char *ReceivedDataFromClient);

/*
Parameters - none.
Returns - none.
Description - checks if game ended and updates server accordingly.
*/
void CheckIfGameEnded();

/*
Parameters - BoardIsFull - to update.
Returns - none.
Discription: This function checks if the board is full and update the Parameter accordingly.
*/
void CheckForFullBoard(bool *BoardIsFull);

/*
Parameters - YellowWon & RedWon - to update.
Returns - none.
Discription: This function checks if there are four identical elements in a matrix either horizontally, vertically, or diagonally and update the Parameters accordingly.
*/
void CheckForWinner(bool *YellowWon, bool *RedWon);

/*
Parameters - OWon: signals if O won, XWon: signals if X won, BoardIsFull: signals if board is full and it's a tie.
Returns - none.
Description - when game is ended, handles the game ended message to send to clients according to game's result.
*/
void HandleGameEndedMessage(bool OWon, bool XWon, bool BoardIsFull);

/*
Parameters - ClientIndex: the index of the client.
Returns - none.
Description - handle message_request reply message to send to client.
*/
void HandleSendMessageToOtherClient(int ClientIndex, char *ReceivedDataFromClient);


// Function Definitions --------------------------------------------------------------------

void WINAPI GameThread(LPVOID lpParam)
{
	if (NULL == lpParam) {
		WriteToLogFile(Server.LogFilePtr, "Custom message: Error in game_connect_four. Received null pointer.\n");
		CloseSocketsThreadsWSAData();
		exit(ERROR_CODE);
	}
	DWORD wait_code;
	BOOL ret_val;
	int *ClientIndexPtr = (int*)lpParam;
	bool UserWasAccepted = HandleNewUserRequestAndAccept(*ClientIndexPtr);
	if (!UserWasAccepted) {
		if (ReleaseOneSemaphore(Server.NumberOfConnectedUsersSemaphore) == FALSE) { // to signal to ConnectUsersThread
			WriteToLogFile(Server.LogFilePtr, "Custom message: failed to release NumberOfConnectedUsers semaphore.\n");
			CloseSocketsThreadsWSAData();
			exit(ERROR_CODE);
		}
		return;
	}
	UpdateNumberOfConnectedUsers();
	
	if (Server.GameStatus == Started) {
		SendGameStartedBoardViewAndTurnSwitch(0);
		SendGameStartedBoardViewAndTurnSwitch(1);
	}

	while (TRUE) { // game started
		char *ReceivedDataFromClient = ReceiveData(Server.ClientsSockets[*ClientIndexPtr], Server.LogFilePtr);
		if (ReceivedDataFromClient == NULL) {
			WriteToLogFile(Server.LogFilePtr, "Custom message: failed with TRNS_FAILED.\n");
			CloseSocketsThreadsWSAData();
			exit(ERROR_CODE);
		}
		if (STRINGS_ARE_EQUAL(ReceivedDataFromClient, "FINISHED")) { 
			Server.NumberOfConnectedUsers -= 1;
			if (Server.GameStatus == NotStarted) {
				CloseOneSocket(Server.ClientsSockets[*ClientIndexPtr], Server.LogFilePtr); // close client socket
				Server.FirstClientDisconnectedBeforeGameStarted = true;
				if (TerminateThread(Server.ConnectUsersThreadHandle, WAIT_OBJECT_0) == FALSE) { // reset for new connect session
					WriteToLogFile(Server.LogFilePtr, "Custom message: Error when terminating ConnectUsersThreadHandle.\n");
					CloseSocketsThreadsWSAData();
					exit(ERROR_CODE);
				}
				break; // finished communication
			}
		}
		if (Server.NumberOfConnectedUsers < NUMBER_OF_CLIENTS && Server.GameStatus == Started) {
			OutputMessageToWindowAndLogFile(Server.LogFilePtr, "Player disconnected. Ending communication.\n");
			shutdown(Server.ClientsSockets[*ClientIndexPtr], SD_BOTH);
			CloseOneSocket(Server.ClientsSockets[*ClientIndexPtr], Server.LogFilePtr); // close client socket
			Server.GameStatus = Ended;
			if (TerminateThread(Server.ConnectUsersThreadHandle, WAIT_OBJECT_0) == FALSE) { // reset for next game
				WriteToLogFile(Server.LogFilePtr, "Custom message: Error when terminating ConnectUsersThreadHandle.\n");
				CloseSocketsThreadsWSAData();
				exit(ERROR_CODE);
			}
			break;
		}
		if (Server.NumberOfConnectedUsers == 0 || Server.GameStatus == Ended) { //closing the second client when exit
			if (TerminateThread(Server.ClientsThreadHandle[*ClientIndexPtr], WAIT_OBJECT_0) == FALSE) { // reset for new connect session
				WriteToLogFile(Server.LogFilePtr, "Custom message: Error when terminating ClientsThreadHandle.\n");
				CloseSocketsThreadsWSAData();
				exit(ERROR_CODE);
			}
				exit(SUCCESS_CODE);
		}
		wait_code = WaitForSingleObject(Server.ServerPropertiesUpdatesMutex, INFINITE); // wait for Mutex access
		if (WAIT_OBJECT_0 != wait_code) {
			WriteToLogFile(Server.LogFilePtr, "Custom message: Error in wait for ServerPropertiesUpdatesMutex.\n");
			CloseSocketsThreadsWSAData();
			exit(ERROR_CODE);
		}

		/* Critical Code Starts */

		HandleReceivedDataFromClient(ReceivedDataFromClient, *ClientIndexPtr); // each time one client is handled
		Sleep(SEND_MESSAGES_WAIT);

		/* Critical Code Ends */

		ret_val = ReleaseMutex(Server.ServerPropertiesUpdatesMutex); // release mutex
		if (FALSE == ret_val) {
			WriteToLogFile(Server.LogFilePtr, "Custom message: Error in releasing ServerPropertiesUpdatesMutex.\n");
			CloseSocketsThreadsWSAData();
			exit(ERROR_CODE);
		}
		if (Server.GameStatus == Ended) {
			for (int Client = 0; Client < NUMBER_OF_CLIENTS; Client++) {
				shutdown(Server.ClientsSockets[Client], SD_SEND);
			}
		}
	}
}


bool HandleNewUserRequestAndAccept(int ClientIndex) {
	char *ReceivedDataFromClient = ReceiveData(Server.ClientsSockets[ClientIndex], Server.LogFilePtr);
	if (ReceivedDataFromClient == NULL) {
		WriteToLogFile(Server.LogFilePtr, "Custom message: failed with TRNS_FAILED.\n");
		CloseSocketsThreadsWSAData();
		exit(ERROR_CODE);
	}
	ParseNewUserRequest(ReceivedDataFromClient, ClientIndex);
	char NewUserReply[MESSAGE_LENGTH];
	bool UserNameIsTaken = (ClientIndex == 1) && (STRINGS_ARE_EQUAL(Server.Players[0].UserName, Server.Players[1].UserName));
	bool UserAccepted;
	if (UserNameIsTaken) {
		strcpy(NewUserReply, "NEW_USER_DECLINED\n");
		UserAccepted = false;
	}
	else {
		Server.Players[ClientIndex].PlayerType = ClientIndex == 0 ? R : Y;
		if (Server.Players[ClientIndex].PlayerType == R) {
			sprintf(NewUserReply, "NEW_USER_ACCEPTED:1\n");
		}
		else {
			sprintf(NewUserReply, "NEW_USER_ACCEPTED:2\n");
		}
		UserAccepted = true;
	}
	int SendDataToServerReturnValue = SendData(Server.ClientsSockets[ClientIndex], NewUserReply, Server.LogFilePtr); 
	if (SendDataToServerReturnValue == ERROR_CODE) {
		CloseSocketsThreadsWSAData();
		exit(ERROR_CODE);
	}
	Sleep(SEND_MESSAGES_WAIT); // give a little time before sending game is on
	char TempMessage[MESSAGE_LENGTH];
	sprintf(TempMessage, "Custom message: Sent %s to Client %d, UserName %s.\n",
		NewUserReply, ClientIndex, Server.Players[ClientIndex].UserName);
	WriteToLogFile(Server.LogFilePtr, TempMessage);
	return UserAccepted;
}


void ParseNewUserRequest(char *ReceivedDataFromClient, int ClientIndex) {
	int StartPosition = 0;
	int EndPosition = 0;
	int ParameterSize;
	while (ReceivedDataFromClient[EndPosition] != ':') { // assuming valid input
		EndPosition++;
	}
	ParameterSize = (EndPosition - 1) - StartPosition + 1;
	if (!FIRST_N_ON_STRINGS_ARE_EQUAL(ReceivedDataFromClient, "NEW_USER_REQUEST", ParameterSize)) {
		char ErrorMessage[MESSAGE_LENGTH];
		sprintf(ErrorMessage, "Custom message: Got unexpected data from client number %d. Exiting...\n", ClientIndex);
		WriteToLogFile(Server.LogFilePtr, ErrorMessage);
		free(ReceivedDataFromClient);
		CloseSocketsThreadsWSAData();
		exit(ERROR_CODE);
	}
	EndPosition++;
	StartPosition = EndPosition;
	EndPosition = FindEndOfDataSymbol(ReceivedDataFromClient);
	ParameterSize = (EndPosition - 1) - StartPosition + 1;
	strncpy(Server.Players[ClientIndex].UserName, ReceivedDataFromClient + StartPosition, ParameterSize);
	Server.Players[ClientIndex].UserName[ParameterSize] = '\0';											
	free(ReceivedDataFromClient);
	char TempMessage[MESSAGE_LENGTH];
	sprintf(TempMessage, "Custom message: Received NEW_USER_REQUEST from Client %d, UserName %s.\n",
		ClientIndex, Server.Players[ClientIndex].UserName);
	WriteToLogFile(Server.LogFilePtr, TempMessage);
}


void UpdateNumberOfConnectedUsers() {
	DWORD wait_code;
	BOOL ret_val;

	wait_code = WaitForSingleObject(Server.ServerPropertiesUpdatesMutex, INFINITE); // wait for Mutex access
	if (WAIT_OBJECT_0 != wait_code) {
		WriteToLogFile(Server.LogFilePtr, "Custom message: Error in wait for ServerPropertiesUpdatesMutex.\n");
		CloseSocketsThreadsWSAData();
		exit(ERROR_CODE);
	}

	/* Critical code starts here */

	Server.NumberOfConnectedUsers++;
	if (Server.NumberOfConnectedUsers == 2) {
		Server.GameStatus = Started;
	}
	if (ReleaseOneSemaphore(Server.NumberOfConnectedUsersSemaphore) == FALSE) { // to signal to ConnectUsersThread
		WriteToLogFile(Server.LogFilePtr, "Custom message: failed to release NumberOfConnectedUsers semaphore.\n");
		CloseSocketsThreadsWSAData();
		exit(ERROR_CODE);
	}

	/* Critical code ends here */

	ret_val = ReleaseMutex(Server.ServerPropertiesUpdatesMutex); // release mutex
	if (FALSE == ret_val) {
		WriteToLogFile(Server.LogFilePtr, "Custom message: Error in releasing ServerPropertiesUpdatesMutex.\n");
		CloseSocketsThreadsWSAData();
		exit(ERROR_CODE);
	}
}


void SendGameStartedBoardViewAndTurnSwitch(int ClientIndex) {
	int SendDataToServerReturnValue = SendData(Server.ClientsSockets[ClientIndex], "GAME_STARTED\n", Server.LogFilePtr);
	if (SendDataToServerReturnValue == ERROR_CODE) {
		WriteToLogFile(Server.LogFilePtr, "Custom message: Error in Sending GAME_STARTED to player.\n");
		CloseSocketsThreadsWSAData();
		exit(ERROR_CODE);
	}
	Sleep(SEND_MESSAGES_WAIT);
	SendBoardView(ClientIndex); 
	Sleep(SEND_MESSAGES_WAIT);
	SendTurn(ClientIndex); 
}


void SendBoardView(int ClientIndex) {
	char BoardMessage[MESSAGE_LENGTH]; 
	int LineOffset, LineLength;
	char CharToUpdate;
	
	strcpy(BoardMessage, "BOARD_VIEW:0000000 0000000 0000000 0000000 0000000 0000000\n"); //row 0 from left.
	LineOffset = BOARD_VIEW_LINE_OFFSET;

	int ColumnNum;
	int RowNum;
	for (ColumnNum = 0; ColumnNum < BOARD_WIDTH; ColumnNum++) {
		for (RowNum = 0; RowNum < BOARD_HEIGHT; RowNum++) {
			if (Server.Board[RowNum][ColumnNum] != None) {
				CharToUpdate = Server.Board[RowNum][ColumnNum] == R ? 'R' : 'Y';
				BoardMessage[LineOffset + ColumnNum + (BOARD_WIDTH+1)*RowNum] = CharToUpdate;
			}
		}
	}
	if (Server.GameStatus == Ended) return;
	int SendDataToServerReturnValue = SendData(Server.ClientsSockets[ClientIndex], BoardMessage, Server.LogFilePtr);
	if (SendDataToServerReturnValue == ERROR_CODE) {
		WriteToLogFile(Server.LogFilePtr, "Custom message: Error in Sending BOARD_VIEW to the client.\n");
		CloseSocketsThreadsWSAData();
		exit(ERROR_CODE);
	}
	char TempMessage[MESSAGE_LENGTH];
	sprintf(TempMessage, "Custom message: Sent BOARD_VIEW to Client %d, UserName %s.\n",
		ClientIndex, Server.Players[ClientIndex].UserName);
	WriteToLogFile(Server.LogFilePtr, TempMessage);
}


void SendTurn(int ClientIndex) { 
	char TurnMessage[MESSAGE_LENGTH];
	char *MessageType = "TURN_SWITCH";
	char *UserNameOfPlayerWhoHasTurn = Server.Turn == R ? Server.Players[0].UserName : Server.Players[1].UserName;

	sprintf(TurnMessage, "%s:%s\n", MessageType, UserNameOfPlayerWhoHasTurn);
	int SendDataToServerReturnValue = SendData(Server.ClientsSockets[ClientIndex], TurnMessage, Server.LogFilePtr); 
	if (SendDataToServerReturnValue == ERROR_CODE) {
		WriteToLogFile(Server.LogFilePtr, "Custom message: Error in Sending TURN_SWITCH to the client.\n");
		CloseSocketsThreadsWSAData();
		exit(ERROR_CODE);
	}
	char TempMessage[MESSAGE_LENGTH];
	sprintf(TempMessage, "Custom message: Sent TURN_SWITCH to Client %d, UserName %s.\n",
		ClientIndex, Server.Players[ClientIndex].UserName);
	WriteToLogFile(Server.LogFilePtr, TempMessage);
}


void HandleReceivedDataFromClient(char *ReceivedDataFromClient, int ClientIndex) { 
	if (FIRST_N_ON_STRINGS_ARE_EQUAL(ReceivedDataFromClient, "PLAY_REQUEST:", PLAY_REQUEST_SIZE)) {
		HandlePlayRequest(ClientIndex, ReceivedDataFromClient);
	}
	else if (FIRST_N_ON_STRINGS_ARE_EQUAL(ReceivedDataFromClient, "SEND_MESSAGE:", SEND_MESSAGE_SIZE)) {
		HandleSendMessageToOtherClient(ClientIndex, ReceivedDataFromClient); 
	}
	else {
		WriteToLogFile(Server.LogFilePtr, "Custom message: Got unexpected answer from client. Exiting...\n");
		free(ReceivedDataFromClient);
		CloseSocketsThreadsWSAData();
		exit(ERROR_CODE);
	}
	char TempMessage[MESSAGE_LENGTH];
	sprintf(TempMessage, "Custom message: Received from client: %s\n", ReceivedDataFromClient);
	WriteToLogFile(Server.LogFilePtr, TempMessage);
	free(ReceivedDataFromClient);
}


void HandlePlayRequest(int ClientIndex, char *ReceivedDataFromClient) {
	char PlayReply[MESSAGE_LENGTH];
	int ColumnIndex = ReceivedDataFromClient[PLAY_REQUEST_SIZE] - '0'; // will give the digit char as int.
	bool PlayWasAccepted = false;

	if (Server.Turn != Server.Players[ClientIndex].PlayerType) {
		strcpy(PlayReply, "PLAY_DECLINED:Not your turn\n");
	}
	else if (Server.GameStatus == NotStarted) {
		strcpy(PlayReply, "PLAY_DECLINED:Game has not started\n");
	}
	else if (ColumnIndex >= BOARD_WIDTH || Server.Board[0][ColumnIndex] != None) { // bigger than number of columns || column if full
		strcpy(PlayReply, "PLAY_DECLINED:Illegal move\n");
	}
	else {
		strcpy(PlayReply, "PLAY_ACCEPTED\n");
		PlayWasAccepted = true;
	}
	int SendDataToServerReturnValue = SendData(Server.ClientsSockets[ClientIndex], PlayReply, Server.LogFilePtr);
	if (SendDataToServerReturnValue == ERROR_CODE) {
		WriteToLogFile(Server.LogFilePtr, "Custom message: Error in Sending PLAY_ACCEPTED/PLAY_DECLINED to the client.\n");
		CloseSocketsThreadsWSAData();
		exit(ERROR_CODE);
	}
	char TempMessage[MESSAGE_LENGTH];
	sprintf(TempMessage, "Custom message: Sent %s to Client %d, UserName %s.\n",
		PlayReply, ClientIndex, Server.Players[ClientIndex].UserName);
	WriteToLogFile(Server.LogFilePtr, TempMessage);

	if (PlayWasAccepted) { // handle play accepted
		int RowIndex;
		for (RowIndex = BOARD_HEIGHT - 1; RowIndex > 0; RowIndex--) // -1 is for the last row. if it got to 4 and won't break -> RowIndex++ will give the right row (5)
		{
			if (Server.Board[RowIndex][ColumnIndex] == None)
				break;
		}
		Server.Board[RowIndex][ColumnIndex] = Server.Players[ClientIndex].PlayerType; // mark play
		Server.Turn = (Server.Turn == R) ? Y : R; // switch turns
		for (int Client = 0; Client < NUMBER_OF_CLIENTS; Client++) {
			SendBoardView(Client);
			Sleep(SEND_MESSAGES_WAIT);
		}
		CheckIfGameEnded();
		if (Server.GameStatus == Started){
			for (int Client = 0; Client < NUMBER_OF_CLIENTS; Client++) {
				SendTurn(Client);
			}
		}
	}
}


void CheckIfGameEnded() { 
	bool YellowWon = false;
	bool RedWon = false;
	bool BoardIsFull = true;

	CheckForFullBoard(&BoardIsFull);
	CheckForWinner(&YellowWon, &RedWon);

	if (YellowWon || RedWon || BoardIsFull) {
		Sleep(SEND_MESSAGES_WAIT); // let turn message pass
		HandleGameEndedMessage(YellowWon, RedWon, BoardIsFull);
		Server.GameStatus = Ended;
		if (TerminateThread(Server.ConnectUsersThreadHandle, WAIT_OBJECT_0) == FALSE) { // reset for next game
			WriteToLogFile(Server.LogFilePtr, "Custom message: Error when terminating ConnectUsersThreadHandle.\n");
			CloseSocketsThreadsWSAData();
			exit(ERROR_CODE);
		}
	}
}


void CheckForWinner(bool *YellowWon, bool *RedWon) 
{
	for (int row = 0; row < BOARD_HEIGHT; row++)
	{
		for (int col = 0; col < BOARD_WIDTH; col++)
		{
			//Current element in the board
			int element = Server.Board[row][col];

			/* If there are 3 elements remaining to the right of the current element's
			   position and the current element equals each of them, then there is a winner */
			if (col <= BOARD_WIDTH - 4 && element == Server.Board[row][col + 1] && element == Server.Board[row][col + 2] && element == Server.Board[row][col + 3] && element != None)
			{
				if (element == R)
					*RedWon = true;
				else
					*YellowWon = true;
				return;
			}
			/* If there are 3 elements remaining below the current element's position
			   and the current element equals each of them, then there is a winner */
			if (row <= BOARD_HEIGHT - 4 && element == Server.Board[row + 1][col] && element == Server.Board[row + 2][col] && element == Server.Board[row + 3][col] && element != None)
			{
				if (element == R)
					*RedWon = true;
				else
					*YellowWon = true;
				return;
			}
			/* If we are in a position in the board such that there are diagonals
			   remaining to the bottom right of the current element, then we check */
			if (row <= BOARD_HEIGHT - 4 && col <= BOARD_WIDTH - 4)
			{
				// If the current element equals each element diagonally to the bottom right we have a winner
				if (element == Server.Board[row + 1][col + 1] && element == Server.Board[row + 2][col + 2] && element == Server.Board[row + 3][col + 3] && element != None)
				{
					if (element == R)
						*RedWon = true;
					else
						*YellowWon = true;
					return;
				}
			}			
			/* If we are in a position in the board such that there are diagonals
			   remaining to the bottom left of the current element, then we check */
			if (row <= BOARD_HEIGHT - 4 && col >= BOARD_WIDTH - 4)
			{
				// If the current element equals each element diagonally to the bottom left we have a winner
				if (element == Server.Board[row + 1][col - 1] && element == Server.Board[row + 2][col - 2] && element == Server.Board[row + 3][col - 3] && element != None)
				{
					if (element == R)
						*RedWon = true;
					else
						*YellowWon = true;
					return;
				}
			}

		}
	}

	// No winner
	return;
}


void CheckForFullBoard(bool *BoardIsFull) 
{
	for (int row = 0; row < BOARD_HEIGHT; row++)
	{
		for (int col = 0; col < BOARD_WIDTH; col++)
		{
			if (Server.Board[row][col] == None)
			{
				*BoardIsFull = false;
				return;
			}
		}
	}
	return;
}


void HandleGameEndedMessage(bool YellowWon, bool RedWon, bool BoardIsFull) {
	char GameEndedMessage[MESSAGE_LENGTH];
	if (RedWon) {
		sprintf(GameEndedMessage, "GAME_ENDED:%s\n", Server.Players[0].UserName);
	}
	else if (YellowWon) {
		sprintf(GameEndedMessage, "GAME_ENDED:%s\n", Server.Players[1].UserName);
	}
	else if (BoardIsFull) {
		strcpy(GameEndedMessage, "GAME_ENDED:Tie\n");
	}
	for (int ClientIndex = 0; ClientIndex < NUMBER_OF_CLIENTS; ClientIndex++) {
		int SendDataToServerReturnValue = SendData(Server.ClientsSockets[ClientIndex], GameEndedMessage, Server.LogFilePtr);
		if (SendDataToServerReturnValue == ERROR_CODE) {
			WriteToLogFile(Server.LogFilePtr, "Custom message: Error in Sending GAME_ENDED to the client.\n");
			CloseSocketsThreadsWSAData();
			exit(ERROR_CODE);
		}
	}
	Sleep(SEND_MESSAGES_WAIT); // so server will send game ended message before closing connections
	char TempMessage[MESSAGE_LENGTH];
	sprintf(TempMessage, "Custom message: Sent %s.\n", GameEndedMessage);
	WriteToLogFile(Server.LogFilePtr, TempMessage);
}


void HandleSendMessageToOtherClient(int ClientIndex, char *ReceivedDataFromClient)
{
	char SendMessageReply[MESSAGE_LENGTH];
	char MessageToOtherClient[MESSAGE_LENGTH];
	sprintf(SendMessageReply, "RECEIVE_MESSAGE:%s;%s", Server.Players[ClientIndex].UserName, ReceivedDataFromClient + SEND_MESSAGE_SIZE);
	int SentDataToServerReturnValue = ClientIndex == 0 ? SendData(Server.ClientsSockets[1], SendMessageReply, Server.LogFilePtr)
		: SendData(Server.ClientsSockets[0], SendMessageReply, Server.LogFilePtr);
	if (SentDataToServerReturnValue == ERROR_CODE) {
		WriteToLogFile(Server.LogFilePtr, "Custom message: Error in Sending MESSAGE_RECIEVED to the client.\n");
		CloseSocketsThreadsWSAData();
		exit(ERROR_CODE);
	}
	char TempMessage[MESSAGE_LENGTH];
	sprintf(TempMessage, "Custom message: Sent MESSAGE_RECIEVED to Client %d, UserName %s.\n",
		ClientIndex, Server.Players[ClientIndex].UserName);
	WriteToLogFile(Server.LogFilePtr, TempMessage);
}
