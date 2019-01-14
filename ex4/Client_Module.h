/*
Authors			- Eli Slavutsky (308882992) & Nisan Tagar (302344031)
Project Name	- Ex4
Description		- Connect 4 - A Server/Client game with human or file mode players
				- Client module handles user cli, socket connection and file mode input
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


	#define RED_PLAYER 1
	#define YELLOW_PLAYER 2
	#define BOARD_HEIGHT 6
	#define BOARD_WIDTH  7
	#define BLACK  15
	#define RED    204
	#define YELLOW 238

	#define MACHINE_DELAY_MS 1500

	// Global Variables ------------------------------------------------------------
	int game_ended;
	int cmd_ready;
	int connected ;		// global flag to know if already connected to server
	int game_started;
	int read_file;
	int my_turn;
	char cmd_to_server		[MAX_MSG_SIZE];
	char server_to_client	[MAX_MSG_SIZE];
	int board[BOARD_HEIGHT][BOARD_WIDTH];
	char my_name			[MAX_MSG_SIZE];
	FILE *client_log;
	FILE *input_file;
	HANDLE  hConsole;
	SOCKET m_socket;

	// Function Declarations -------------------------------------------------------
	void MainClient(int argc, char *argv[]);
	void PrintBoard(int board[][BOARD_WIDTH], HANDLE consoleHandle);
	static DWORD RecvDataThread(void);
	static DWORD SendDataThread(void);
	static DWORD player_input(void);
	static DWORD file_input(LPVOID lpParam);

#endif // SOCKET_EXAMPLE_CLIENT_H