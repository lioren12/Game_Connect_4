// connect_clients.h

/*
Description - declaration of the functions that are used to connect clients to the server and needed define.
*/

#ifndef CONNECT_CLIENTS_H
#define CONNECT_CLIENTS_H

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

// Library Includes --------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <winsock2.h>
#include <windows.h>

// Project Includes --------------------------------------------------------------------

#include "handle_server.h"
#include "game_connect_four.h"
#include "socket.h"

// Macros & Definitions --------------------------------------------------------------------

#define ERROR_CODE ((int) (-1))
#define SUCCESS_CODE ((int) (0))
#define MINUTE_IN_MS 60000

// Declarations --------------------------------------------------------------------

/*
Parameters - none.
Returns - none.
Description - handle the connection of clients to the server.
*/
void HandleConnectToClients();

#endif // CONNECT_CLIENTS_H

