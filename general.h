// general.h

/*
Description - declaration of general functions and defines used by both server and client.
*/

#ifndef GENERAL_H
#define GENERAL_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

// Library Includes --------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <winsock2.h>
#include <WS2tcpip.h>

// Macros & Definitions --------------------------------------------------------------------

#pragma comment(lib, "Ws2_32.lib")

#define ERROR_CODE ((int) (-1))
#define SUCCESS_CODE ((int) (0))
#define BOARD_HEIGHT 6
#define BOARD_WIDTH 7

#define MESSAGE_LENGTH 100               //included the enter on the end of the message
#define SEND_MESSAGES_WAIT 20   
#define SEND_MESSAGES_FILE_WAIT 1000
#define GAME_ENDED_MESSAGE_WAIT 2500
#define USER_NAME_LENGTH 30    
#define INPUT_MODE_LENGTH 5

#define FIRST_N_ON_STRINGS_ARE_EQUAL( Str1, Str2, n ) ( strncmp( (Str1), (Str2),(n) ) == 0 )
#define STRINGS_ARE_EQUAL( Str1, Str2 ) ( strcmp( (Str1), (Str2) ) == 0 )

// Declarations --------------------------------------------------------------------

typedef enum _PlayerType {
	None,
	R,	   // RED
	Y      // YELLOW
}PlayerType;

/*
Parameters - LogFilePathPtr: log file path in case of an error, MessageToWrite: message to write to log.
Returns - none.
Description - writes the input message to screen and log file.
*/
void OutputMessageToWindowAndLogFile(char *LogFilePathPtr, char *MessageToWrite);

/*
Parameters - none.
Returns - none.
Description - init wsa data.
*/
void InitWsaData();

/*
Parameters - LogFilePathPtr: log file path in case of an error.
Returns - none.
Description - init log file.
*/
void InitLogFile(char *LogFilePathPtr);

/*
Parameters - LogFilePathPtr: log file path in case of an error, MessageToWrite: message to write to log.
Returns - none.
Description - writes the input message to log file.
*/
void WriteToLogFile(char *LogFilePathPtr, char *MessageToWrite);

/*
Parameters - p_start_routine - a pointer to the function to be executed by the thread,
			 p_thread_id - a pointer to a variable that receives the thread identifier (output parameter),
			 p_thread_argument - the argument to send to thread's function.
Returns - if the function succeeds, the return value is a handle to the new thread, if not prints error and returns NULL.
Description - creating the thread with the right parameters.
*/
HANDLE CreateThreadSimple(LPTHREAD_START_ROUTINE p_start_routine, LPVOID p_thread_parameters, LPDWORD p_thread_id,
	char *LogFilePathPtr);

/*
Parameters - Semaphore: the semaphore to be released.
Returns - TRUE if succeeded, FALSE if failed.
Description - release one semaphore.
*/
BOOL ReleaseOneSemaphore(HANDLE Semaphore);

/*
Parameters - HandleToClose: the handle to be closed, LogFilePathPtr: log file path in case of an error.
Returns - none.
Description - close one thread handle.
*/
void CloseOneThreadHandle(HANDLE HandleToClose, char *LogFilePathPtr);

/*
Parameters - LogFilePathPtr: log file path in case of an error.
Returns - none.
Description - close wsa data.
*/
void CloseWsaData(char *LogFilePathPtr);

/*
Parameters - ReceivedData: the data that was received from server.
Returns - the end position of the parsed parameter.
Description - searches and updates the end position of the parsed parameter in ReceivedData.
*/
int FindEndOfDataSymbol(char *ReceivedData);

#endif // GENERAL_H