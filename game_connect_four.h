// game_connect_four.h

/*
Description - Description - declaration of the thread function that runs the game and needed defines.
*/

#ifndef GAME_CONNECT_FOUR_H
#define GAME_CONNECT_FOUR_H

// Library Includes --------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <winsock2.h>
#include <windows.h>

// Project Includes --------------------------------------------------------------------

#include "socket.h"
#include "handle_server.h"
#include "general.h"

// Macros & Definitions --------------------------------------------------------------------

#define ERROR_CODE ((int) (-1))
#define RED_PLAYER 1
#define YELLOW_PLAYER 2
#define BLACK  15
#define RED    204
#define YELLOW 238
#define BOARD_VIEW_LINE_OFFSET 11
#define BOARD_VIEW_LINE_LENGTH 26
#define PLAY_REQUEST_SIZE 13
#define SEND_MESSAGE_SIZE 13
#define MESSAGE_RECIEVED_SIZE 17
#define NUM_OF_SAME_COLOUR_FOR_WINNING 4
#define RED_LENGTH 3
#define NUMBER_OF_ROWS_FOR_DIAGONAL_WINNING 3
#define NUMBER_OF_COLUMNS_FOR_DIAGONAL_WINNING 4

// Declarations --------------------------------------------------------------------

/*
Parameters - lpParam: pointer to ClientIndex.
Returns - none.
Description - handles the entire operation of the server during the game.
*/
void WINAPI GameThread(LPVOID lpParam);

#endif // GAME_CONNECT_FOUR_H
