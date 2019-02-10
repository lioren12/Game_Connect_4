// handle_server.c

/*
Description - implementation of server operation - initiating server, creating and handling sockets, handling games
			  and closing all used resources when server finishes.
*/

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

// Project Includes --------------------------------------------------------------------

#include "handle_server.h"

// Declarations --------------------------------------------------------------------

/*
Parameters - argv: contains the log file path and port to connect to.
Returns - none.
Description - initialize the database of the server.
*/
void InitServer(char *argv[]);

/*
Parameters - none.
Returns - none.
Description - initialize the database of the server for new game when current game ends.
*/
void InitServerForNewGame();

/*
Parameters - none.
Returns - none.
Description - runs the game for the defined number of games.
*/
void HandleServer();

/*
Parameters - none.
Returns - none.
Description - handles the creation, binding and listening of the socket that listens for connections.
*/
void CreateSocketBindAndListen();

/*
Parameters - none.
Returns - none.
Description - binding of the socket that listens for connections.
*/
void SetSockAddrInAndBind();

/*
Parameters - none.
Returns - none.
Description - setting the socket that listens for connections to listen.
*/
void SetSocketToListen();

/*
Parameters - none.
Returns - none.
Description - clears the game board.
*/
void InitBoard();

// Function Definitions --------------------------------------------------------------------

void MainServer(char *argv[]) 
{
	InitServer(argv);
	HandleServer();
	CloseSocketsThreadsWSAData();
}

void InitServer(char *argv[]) { 
	Server.ListeningSocket = INVALID_SOCKET;
	int ClientIndex = 0;

	for (; ClientIndex < NUMBER_OF_CLIENTS; ClientIndex++) {
		Server.ClientsSockets[ClientIndex] = INVALID_SOCKET;
		Server.ClientsThreadHandle[ClientIndex] = NULL;
		Server.Players[ClientIndex].ClientIndex = ClientIndex;
	}
	Server.ConnectUsersThreadHandle = NULL;
	//argv[1] is for state: server/client
	Server.LogFilePtr = argv[2];
	Server.PortNum = atoi(argv[3]);
	InitLogFile(Server.LogFilePtr);

	Server.NumberOfConnectedUsersSemaphore = CreateSemaphore(
		NULL,	/* Default security attributes */
		0,		/* Initial Count - not signaled */
		1,		/* Maximum Count */
		NULL);	/* un-named */
	if (Server.NumberOfConnectedUsersSemaphore == NULL) {
		WriteToLogFile(Server.LogFilePtr, "Custom message: Error when creating NumberOfConnectedUsers semaphore.\n");
		exit(ERROR_CODE);
	}

	Server.ServerPropertiesUpdatesMutex = CreateMutex(
		NULL,	/* default security attributes */
		FALSE,	/* initially not owned */
		NULL);	/* unnamed mutex */
	if (NULL == Server.ServerPropertiesUpdatesMutex) {
		WriteToLogFile(Server.LogFilePtr, "Custom message: Error when creating ServerPropertiesUpdates mutex.\n");
		exit(ERROR_CODE);
	}
}


void HandleServer() {
	DWORD wait_code;
	int GameIndex = 0;

	CreateSocketBindAndListen();

	for (; GameIndex < NUMBER_OF_GAMES; GameIndex++) {
		InitServerForNewGame();
		HandleConnectToClients();

		wait_code = WaitForMultipleObjects(NUMBER_OF_CLIENTS, Server.ClientsThreadHandle, TRUE, INFINITE);
		if (WAIT_OBJECT_0 != wait_code) {
			WriteToLogFile(Server.LogFilePtr, "Custom message: Error when waiting for program to end.\n");
			CloseSocketsThreadsWSAData();
			exit(ERROR_CODE);
		}
	}
}


void CreateSocketBindAndListen() {
	InitWsaData();
	Server.ListeningSocket = CreateOneSocket();
	if (Server.ListeningSocket == INVALID_SOCKET) {
		char ErrorMessage[MESSAGE_LENGTH];
		sprintf(ErrorMessage, "Custom message: CreateSocketBindAndListen failed to create socket. Error Number is %d\n", WSAGetLastError());
		WriteToLogFile(Server.LogFilePtr, ErrorMessage);
		exit(ERROR_CODE);
	}

	SetSockAddrInAndBind();
	SetSocketToListen();
}


void SetSockAddrInAndBind() {
	int BindingReturnValue;

	Server.ListeningSocketService.sin_family = AF_INET;
	Server.ListeningSocketService.sin_addr.s_addr = inet_addr(SERVER_ADDRESS_STR);
	Server.ListeningSocketService.sin_port = htons(Server.PortNum);
	BindingReturnValue = bind(Server.ListeningSocket, (SOCKADDR*)&Server.ListeningSocketService,
		sizeof(Server.ListeningSocketService));
	if (BindingReturnValue != BINDING_SUCCEEDED) {
		CloseSocketsThreadsWSAData();
		exit(ERROR_CODE);
	}
}


void SetSocketToListen() {
	int ListenReturnValue;
	ListenReturnValue = listen(Server.ListeningSocket, NUMBER_OF_CLIENTS);
	if (ListenReturnValue != LISTEN_SUCCEEDED) {
		char ErrorMessage[MESSAGE_LENGTH];
		sprintf(ErrorMessage, "Custom message: SetSocketToListen failed to set Socket to listen. Error Number is %d\n", WSAGetLastError());
		WriteToLogFile(Server.LogFilePtr, ErrorMessage);
		CloseSocketsThreadsWSAData();
		exit(ERROR_CODE);
	}
}


void InitServerForNewGame() {
	int ClientIndex = 0;

	InitBoard(); // init for new game
	Server.NumberOfConnectedUsers = 0;
	Server.Turn = R;
	Server.GameStatus = NotStarted;
	Server.FirstClientDisconnectedBeforeGameStarted = false;

	for (; ClientIndex < NUMBER_OF_CLIENTS; ClientIndex++) {
		Server.Players[ClientIndex].PlayerType = None;
		strcpy(Server.Players[ClientIndex].UserName, ""); 
	}
}


void InitBoard() {
	int RowNum = 0;
	for (; RowNum < BOARD_HEIGHT; RowNum++) {
		for (int ColumnNum = 0; ColumnNum < BOARD_WIDTH; ColumnNum++) {
			Server.Board[RowNum][ColumnNum] = None;
		}
	}
}


void CloseSocketsThreadsWSAData() {
	int ClientIndex = 0;
	for (; ClientIndex < NUMBER_OF_CLIENTS; ClientIndex++) {
		CloseOneSocket(Server.ClientsSockets[ClientIndex], Server.LogFilePtr);
		CloseOneThreadHandle(Server.ClientsThreadHandle[ClientIndex], Server.LogFilePtr);
	}
	CloseOneThreadHandle(Server.ServerPropertiesUpdatesMutex, Server.LogFilePtr);
	CloseOneSocket(Server.ListeningSocket, Server.LogFilePtr);
	CloseWsaData(Server.LogFilePtr);
}