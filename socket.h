//socket.h

/*
Description - declaration of general functions of send/receive and closing sockets used by both server and client.
			  also needed defines.
*/

#ifndef SOCKET_H
#define SOCKET_H

#define _CRT_SECURE_NO_WARNINGS

// Library Includes --------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <winsock2.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")

// Project Includes --------------------------------------------------------------------

#include "general.h"

// Macros & Definitions --------------------------------------------------------------------

#define ERROR_CODE ((int) (-1)) 
#define SEND_RECEIVE_FLAGS 0
#define RECV_FINISHED 0 

// Declarations --------------------------------------------------------------------

/*
Parameters - Socket: the socket that is sending, DataToSend: the data to send, LogFilePathPtr: log file path in case of an error.
Returns - SUCCESS_CODE if sending succeeded, else ERROR_CODE.
Description - sending data through socket.
*/
int SendData(SOCKET Socket, char *DataToSend, char *LogFilePathPtr);

/*
Parameters - Socket: the socket that is receiving, LogFilePathPtr: log file path in case of an error.
Returns - the received data.
Description - receive data through the socket and return it.
*/
char *ReceiveData(SOCKET Socket, char *LogFilePathPtr);

/*
Parameters - none.
Returns - a new created socket.
Description - creates one socket and returns it.
*/
SOCKET CreateOneSocket();

/*
Parameters - Socket: the socket to be closed, LogFilePathPtr: log file path in case of an error.
Returns - none.
Description - closing one socket.
*/
void CloseOneSocket(SOCKET Socket, char *LogFilePathPtr);

#endif // SOCKET_H
