// handle_server.h

/*
Description - declaration of global functions of the server.
			  defining the structures that the server uses in the programs.
			  creating the Server instance of ServerProperties as a general database variable with all needed information.
*/

#ifndef HANDLE_SERVER_H
#define HANDLE_SERVER_H

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

// Library Includes --------------------------------------------------------------------

#include <stdbool.h>

// Project Includes --------------------------------------------------------------------

#include "general.h"
#include "game_connect_four.h"
#include "socket.h"
#include "connect_clients.h"

// Macros & Definitions --------------------------------------------------------------------

#define ERROR_CODE ((int) (-1))
#define SERVER_ADDRESS_STR "127.0.0.1"
#define NUMBER_OF_CLIENTS 2
#define BINDING_SUCCEEDED 0
#define LISTEN_SUCCEEDED 0
#define NUMBER_OF_GAMES INFINITE

// Declarations --------------------------------------------------------------------

typedef struct _PlayerProperties {
	char UserName[USER_NAME_LENGTH];
	PlayerType PlayerType; 
	int ClientIndex; // to send to thread function
}PlayerProperties;

typedef enum _GameStatus {
	NotStarted,
	Started,
	Ended
}GameStatus;

typedef struct _ServerProperties {
	////// sockets
	SOCKET ListeningSocket; // the socket that is listening to client connections
	SOCKADDR_IN ListeningSocketService;
	SOCKET ClientsSockets[NUMBER_OF_CLIENTS + 1]; // the sockets that are connected to each client. extra one for declining more clients.

	////// threads
	HANDLE ConnectUsersThreadHandle; // thread handle for connecting two users
	DWORD ConnectUsersThreadID; // thread id to the above thread handle
	HANDLE ClientsThreadHandle[NUMBER_OF_CLIENTS]; // thread handle to each client
	DWORD ClientsThreadID[NUMBER_OF_CLIENTS]; // thread id to each client

	////// semaphores and mutexes
	HANDLE NumberOfConnectedUsersSemaphore; // semaphore to signal ConnectUsersThread that NumberOfConnectedUsers was updated.
	HANDLE ServerPropertiesUpdatesMutex; // mutex to update server properties

	////// others
	char *LogFilePtr; // path to log file
	int PortNum; // server's port num
	PlayerProperties Players[NUMBER_OF_CLIENTS];
	int NumberOfConnectedUsers;
	PlayerType Turn; 
	PlayerType Board[BOARD_HEIGHT][BOARD_WIDTH];
	GameStatus GameStatus;
	bool FirstClientDisconnectedBeforeGameStarted;
}ServerProperties;

ServerProperties Server;


/*
Parameters - none.
Returns - none.
Description - closes all used resources when server program finishes / an error occurs.
*/
void CloseSocketsThreadsWSAData();

/*
Parameters - argv[].
Returns - None.
Description - Main operation of Server.
*/
void MainServer(char *argv[]);
#endif // HANDLE_SERVER_H

