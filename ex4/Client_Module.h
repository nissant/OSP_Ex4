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

	#define PLAY_REQUEST		"PLAY_REQUEST"
	#define NEW_USER_REQUEST	"NEW_USER_REQUEST"

	// Global Variables ------------------------------------------------------------
	int game_ended = 0;
	int cmd_ready = 0;
	int connected = 0;		// global flag to know if already connected to server
	char cmd_to_server[MAX_MSG_SIZE];





	SOCKET m_socket;
	// Function Declarations -------------------------------------------------------
	void MainClient(int argc, char *argv[]);
#endif // SOCKET_EXAMPLE_CLIENT_H