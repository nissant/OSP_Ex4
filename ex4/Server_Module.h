/*oOoOoOoOoOo/*
Authors			- Eli Slavutsky (308882992) & Nisan Tagar (302344031)
Project Name	- Ex4
Description		- 
*/

// Includes --------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <winsock2.h>


#ifndef SOCKET_EXAMPLE_SERVER_H
#define SOCKET_EXAMPLE_SERVER_H
	// Defines ---------------------------------------------------------------------
	#define _CRT_SECURE_NO_WARNINGS
	#define _WINSOCK_DEPRECATED_NO_WARNINGS
	#define NUM_OF_WORKER_THREADS 2
	#define MAX_LOOPS 3
	#define SEND_STR_SIZE 35
	#define SERVER_ADDRESS_STR "127.0.0.1"
	#define SERVER_PORT 2345
	#define STRINGS_ARE_EQUAL( Str1, Str2 ) ( strcmp( (Str1), (Str2) ) == 0 )
	// Global Variables ------------------------------------------------------------

	HANDLE ThreadHandles[NUM_OF_WORKER_THREADS];
	SOCKET ThreadInputs[NUM_OF_WORKER_THREADS];

	// Function Declarations -------------------------------------------------------
	void MainServer(char *argv[]);
	static int FindFirstUnusedThreadSlot();
	static void CleanupWorkerThreads();
	static DWORD ServiceThread(SOCKET *t_socket);

#endif // SOCKET_EXAMPLE_SERVER_H