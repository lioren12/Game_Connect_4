// main.c
/*
Authors - Lior Oren (200430239) & Itay Niv (206811309)
Project Name - ex4
Description - main file that checks num of args, and calls MainServer or MainClient to handle program's logic.
*/

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

// Project Includes --------------------------------------------------------------------
#include "handle_server.h"
#include "handle_client.h"  

// Macros & Definitions --------------------------------------------------------------------

#define NUM_OF_ARGS_SERVER 4
#define NUM_OF_ARGS_CLIENT 6

// Functions Definitions --------------------------------------------------------

int main(int argc, char *argv[])
{
	if (argc == NUM_OF_ARGS_SERVER)
		MainServer(argv);

	else if (argc == NUM_OF_ARGS_CLIENT || argc == NUM_OF_ARGS_CLIENT - 1) // -1 for case of human mode (no input file)
		MainClient(argv);

	else
	{
		printf("Not the right amount of input arguments.\nNeed to give three/five/six.\nExiting...\n");
		return ERROR_CODE;
	}
}