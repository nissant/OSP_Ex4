/*oOoOoO/*
Authors			- Eli Slavutsky (308882992) & Nisan Tagar (302344031)
Project Name	- Ex4
Description		- 
*/

// Includes --------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#ifndef SOCKET_EXAMPLE_CLIENT_H
#define SOCKET_EXAMPLE_CLIENT_H	
	// Defines ---------------------------------------------------------------------
	#define _WINSOCK_DEPRECATED_NO_WARNINGS
	#define EXIT_ERROR -1 

	#define PLAY_REQUEST_STR		"PLAY_REQUEST"
	#define NEW_USER_REQUEST_STR	"NEW_USER_REQUEST"
	#define SEND_MESSAGE_STR		"SEND_MESSAGE"

	// Global Variables ------------------------------------------------------------
	int game_ended;
	int cmd_ready;
	int connected ;		// global flag to know if already connected to server
	char cmd_to_server		[MAX_MSG_SIZE];
	char server_to_client	[MAX_MSG_SIZE];
	int board[6][7];
	FILE *client_log;

	


	SOCKET m_socket;
	// Function Declarations -------------------------------------------------------
	void MainClient(int argc, char *argv[]);

#endif // SOCKET_EXAMPLE_CLIENT_H