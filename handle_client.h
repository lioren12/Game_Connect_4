// handle_client.h

/*
Description - declaration of global functions of the client.
			  defining the structures that the client uses in the programs.
			  creating the Client instance of ClientProperties as a general database variable with all needed information.
*/

#ifndef HANDLE_CLIENT_H
#define HANDLE_CLIENT_H

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

// Library Includes --------------------------------------------------------------------

#include <stdbool.h>

// Project Includes --------------------------------------------------------------------

#include "general.h"
#include "handle_server.h"
#include "send_receive.h"
#include "socket.h"
#include "user_interface.h"

// Macros & Definitions --------------------------------------------------------------------

#define ERROR_CODE ((int) (-1))
#define SUCCESS_CODE ((int) (0))
#define NUMBER_OF_THREADS_TO_HANDLE_CLIENT 3 // one for send, one for receive, one for user interface

// Globals --------------------------------------------------------------------

bool IsGameStarted;
PlayerType WhosTurn;

// Declarations --------------------------------------------------------------------

typedef struct ClientProperties {
	////// sockets
	SOCKET Socket; // the socket that is connecting to the server
	SOCKADDR_IN SocketService;

	////// threads
	HANDLE ThreadHandles[NUMBER_OF_THREADS_TO_HANDLE_CLIENT];
	DWORD ThreadIDs[NUMBER_OF_THREADS_TO_HANDLE_CLIENT];

	////// semaphores and mutexes
	HANDLE UserInterfaceSemaphore; // semaphore to block user interface thread until connection is established and user is accepted
	HANDLE SendToServerSemaphore; // semaphore to signal each time a new message to send to server is available

	////// others
	char *LogFilePtr; // path to log file
	FILE *InputFilePtr; // Path to input file
	char *ServerIP; // server's IP 
	int ServerPortNum; // server's port num
	char *InputMode; // file or human
	char UserName[USER_NAME_LENGTH]; 
	PlayerType PlayerType;
	char MessageToSendToServer[MESSAGE_LENGTH]; // each time SendToServerSemaphore is signaled contains the message to send
	bool GotExitFromUserOrGameFinished;
	HANDLE  hConsole;
	BOOL Is_hConsole;
}ClientProperties;

ClientProperties Client;


/*
Parameters - argv[].
Returns - None.
Description - Main operation of Client.
*/
void MainClient(char *argv[]);

/*
Parameters - argv: contains the log file path, port to connect to and the servers ip.
Returns - none.
Description - initialize the database of the client.
*/
void InitClient(char *argv[]);

/*
Parameters - none.
Returns - none.
Description - connects to server and runs the client's threads.
*/
void HandleClient();

/*
Parameters - none.
Returns - none.
Description - closes all used resources when client program finishes / an error occurs.
*/
void CloseSocketThreadsAndWSAData();

#endif // HANDLE_CLIENT_H
