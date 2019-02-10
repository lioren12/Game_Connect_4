// user_interface.h

/*
Description - declaration of the user interface thread function and needed define.
*/

#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

// Project Includes --------------------------------------------------------------------

#include "handle_client.h"
#include "general.h"

// Macros & Definitions --------------------------------------------------------------------

#define USER_INPUT_LENGTH 100
#define STRINGS_ARE_EQUAL( Str1, Str2 ) ( strcmp( (Str1), (Str2) ) == 0 )

// Declarations --------------------------------------------------------------------

/*
Parameters - none.
Returns - none.
Description - the thread function that handles user interface.
*/
void WINAPI UserInterfaceThread();

#endif // USER_INTERFACE_H