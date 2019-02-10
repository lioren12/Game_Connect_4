// handle_client.c

/*
Description - implementation of client operation - initiating client, connecting to server, creating and starting
			  send/receive/user interface threads, and closing all used resources when program finishes.
*/

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

// Project Includes --------------------------------------------------------------------

#include "handle_client.h"

// Declarations --------------------------------------------------------------------

/*
Parameters - none.
Returns - none.
Description - sets the client socket and connects to server.
*/
void ConnectToServer();

/*
Parameters - none.
Returns - none.
Description - creates the thread handles and semaphores used by the client.
*/
void CreateThreadsAndSemaphores();


// Function Definitions --------------------------------------------------------------------

void MainClient(char *argv[])
{
	InitClient(argv);
	HandleClient();
	CloseSocketThreadsAndWSAData();
	return SUCCESS_CODE;
}


void InitClient(char *argv[]) {
	int ThreadIndex = 0;

	Client.Socket = INVALID_SOCKET;
	// argv[1] is for state: server/client
	Client.LogFilePtr = argv[2];
	Client.ServerPortNum = atoi(argv[3]);
	Client.InputMode = argv[4];
	if (STRINGS_ARE_EQUAL(Client.InputMode, "file"))
	{
		Client.InputFilePtr = fopen(argv[5], "r");
		if (Client.InputFilePtr == NULL)
		{
			WriteToLogFile(Client.LogFilePtr, "Custom message: Error in open input file.\n");
			exit(ERROR_CODE);
		}
	}
	// Username request
	if (STRINGS_ARE_EQUAL(Client.InputMode, "human")) {
		printf("Enter your username: ");
		scanf("%s", Client.UserName);  
	}
	else {
		fscanf(Client.InputFilePtr, "%s", Client.UserName);
	}

	for (; ThreadIndex < NUMBER_OF_THREADS_TO_HANDLE_CLIENT; ThreadIndex++) {
		Client.ThreadHandles[ThreadIndex] = NULL;
	}
	Client.ServerIP = SERVER_ADDRESS_STR;
	Client.UserInterfaceSemaphore = NULL;
	Client.SendToServerSemaphore = NULL;
	Client.PlayerType = None;
	Client.GotExitFromUserOrGameFinished = false;
	InitLogFile(Client.LogFilePtr);
	Client.Is_hConsole = FALSE;
	IsGameStarted = FALSE;
	Client.hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
}

void HandleClient() { 
	DWORD wait_code;

	InitWsaData();

	Client.Socket = CreateOneSocket();
	if (Client.Socket == INVALID_SOCKET) {
		char ErrorMessage[MESSAGE_LENGTH];
		sprintf(ErrorMessage, "Custom message: HandleClient failed to create socket.\nError Number is %d\n", WSAGetLastError());
		WriteToLogFile(Client.LogFilePtr, ErrorMessage);
		CloseWsaData(Client.LogFilePtr); 
		exit(ERROR_CODE);
	}

	ConnectToServer();
	CreateThreadsAndSemaphores();

	wait_code = WaitForMultipleObjects(NUMBER_OF_THREADS_TO_HANDLE_CLIENT, Client.ThreadHandles, TRUE, INFINITE);
	if (WAIT_OBJECT_0 != wait_code) {
		WriteToLogFile(Client.LogFilePtr, "Custom message: Error when waiting for program to end.\n");
		CloseSocketThreadsAndWSAData();
		exit(ERROR_CODE);
	}
}


void ConnectToServer() {  
	int ConnectReturnValue;
	char ConnectMessage[MESSAGE_LENGTH];

	Client.SocketService.sin_family = AF_INET;
	Client.SocketService.sin_addr.s_addr = inet_addr(Client.ServerIP);
	Client.SocketService.sin_port = htons(Client.ServerPortNum);

	ConnectReturnValue = connect(Client.Socket, (SOCKADDR*)&Client.SocketService, sizeof(Client.SocketService));
	if (ConnectReturnValue == SOCKET_ERROR) {
		sprintf(ConnectMessage, "Failed connecting to server on %s:%d. Exiting.\n", Client.ServerIP, Client.ServerPortNum);
		OutputMessageToWindowAndLogFile(Client.LogFilePtr, ConnectMessage);
		CloseSocketThreadsAndWSAData();
		exit(ERROR_CODE);
	}
	sprintf(ConnectMessage, "Connected to server on %s:%d\n", Client.ServerIP, Client.ServerPortNum);
	OutputMessageToWindowAndLogFile(Client.LogFilePtr, ConnectMessage);
}


void CreateThreadsAndSemaphores() {
	Client.ThreadHandles[0] = CreateThreadSimple((LPTHREAD_START_ROUTINE)SendThread,  
		NULL,
		&Client.ThreadIDs[0],
		Client.LogFilePtr);

	Client.ThreadHandles[1] = CreateThreadSimple((LPTHREAD_START_ROUTINE)ReceiveThread,   
		NULL,
		&Client.ThreadIDs[1],
		Client.LogFilePtr);

	Client.ThreadHandles[2] = CreateThreadSimple((LPTHREAD_START_ROUTINE)UserInterfaceThread,   
		NULL,
		&Client.ThreadIDs[2],
		Client.LogFilePtr);

	if (Client.ThreadHandles[0] == NULL || Client.ThreadHandles[1] == NULL || Client.ThreadHandles[2] == NULL) {
		WriteToLogFile(Client.LogFilePtr, "Custom message: CreateThreadsAndSemaphores failed to create threads.\n");
		CloseSocketThreadsAndWSAData();
		exit(ERROR_CODE);
	}

	Client.UserInterfaceSemaphore = CreateSemaphore(
		NULL,	/* Default security attributes */
		0,		/* Initial Count - not signaled */
		1,		/* Maximum Count */
		NULL);	/* un-named */

	if (Client.UserInterfaceSemaphore == NULL) {
		WriteToLogFile(Client.LogFilePtr, "Custom message: CreateThreadsAndSemaphores - Error when creating UserInterface semaphore.\n");
		CloseSocketThreadsAndWSAData();
		exit(ERROR_CODE);
	}
	Client.SendToServerSemaphore = CreateSemaphore(
		NULL,	/* Default security attributes */
		0,		/* Initial Count - not signaled */
		1,		/* Maximum Count */
		NULL);	/* un-named */

	if (Client.SendToServerSemaphore == NULL) {
		WriteToLogFile(Client.LogFilePtr, "Custom message: CreateThreadsAndSemaphores - Error when creating SendToServer semaphore.\n");
		CloseSocketThreadsAndWSAData();
		exit(ERROR_CODE);
	}
}

void CloseSocketThreadsAndWSAData() { 
	int ThreadIndex = 0;
	CloseOneSocket(Client.Socket, Client.LogFilePtr);
	for (; ThreadIndex < NUMBER_OF_THREADS_TO_HANDLE_CLIENT; ThreadIndex++) {
		CloseOneThreadHandle(Client.ThreadHandles[ThreadIndex], Client.LogFilePtr);
	}
	CloseOneThreadHandle(Client.UserInterfaceSemaphore, Client.LogFilePtr);
	CloseOneThreadHandle(Client.SendToServerSemaphore, Client.LogFilePtr);
	if (Client.Is_hConsole)
		CloseHandle(Client.hConsole); 
	CloseWsaData(Client.LogFilePtr);
}

