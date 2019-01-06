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
	#define PLAY_REQUEST "PLAY_REQUEST" 
	// Global Variables ------------------------------------------------------------

	SOCKET m_socket;
	// Function Declarations -------------------------------------------------------
	void MainClient(int argc, char *argv[]);
#endif // SOCKET_EXAMPLE_CLIENT_H