// send_receive.h

/*
Description - declaration of the send / receive threads functions and needed defines.
*/

#ifndef SEND_RECEIVE_H
#define SEND_RECEIVE_H

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

// Project Includes --------------------------------------------------------------------

#include "handle_client.h"
#include "socket.h"
#include "general.h"
#include "game_connect_four.h"

// Macros & Definitions --------------------------------------------------------------------

#define GAME_STARTED_SIZE 12
#define TURN_SWITCH_SIZE 12
#define RECEIVE_MESSAGE_SIZE 16           
#define BOARD_VIEW_SIZE 11
#define PLAY_ACCEPTED_SIZE 13
#define PLAY_DECLINED_SIZE 14
#define USER_LIST_REPLY_SIZE 16
#define NEW_USER_ACCEPTED_OR_DECLINED_SIZE 17
#define GAME_ENDED_SIZE 11


// Declarations --------------------------------------------------------------------

/*
Parameters - none.
Returns - none.
Description - the thread function that handles sending data to server.
*/
void WINAPI SendThread();

/*
Parameters - none.
Returns - none.
Description - the thread function that handles received data from server.
*/
void WINAPI ReceiveThread();

#endif // SEND_RECEIVE_H









