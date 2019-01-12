/*
Authors			- Eli Slavutsky (308882992) & Nisan Tagar (302344031)
Project Name	- Ex4
Description		- 
*/

// Includes --------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <stdbool.h>


#ifndef SOCKET_EXAMPLE_SERVER_H
#define SOCKET_EXAMPLE_SERVER_H
	// Defines ---------------------------------------------------------------------
	#define ERROR_CODE -1
	#define SUCCESS_CODE 0
	#define _CRT_SECURE_NO_WARNINGS
	#define _WINSOCK_DEPRECATED_NO_WARNINGS
	#define NUM_OF_WORKER_THREADS 2
	#define MAX_LOOPS 3
	#define SERVER_ADDRESS_STR "127.0.0.1"
	#define STRINGS_ARE_EQUAL( Str1, Str2 ) ( strcmp( (Str1), (Str2) ) == 0 )
	#define MAX_MSG_SIZE 100
	#define MAX_NAME_SIZE 30

	// Message Protocol
	#define NEW_USER_REQUEST 1
	#define NEW_USER_ACCEPTED 2
	#define NEW_USER_DECLINED 3
	#define GAME_STARTED 4
	#define BOARD_VIEW 5
	#define TURN_SWITCH 6
	#define PLAY_REQUEST 7
	#define PLAY_ACCEPTED 8
	#define PLAY_DECLINED 9
	#define GAME_ENDED 10
	#define SEND_MESSAGE 11
	#define RECEIVE_MESSAGE 12
	#define BAD_MSG			99

	// Game
	#define RED_PLAYER 1
	#define YELLOW_PLAYER 2

	#define BOARD_HEIGHT 6
	#define BOARD_WIDTH  7
	

	typedef struct p {
		bool accepted;
		bool myTurn;
		char name[MAX_NAME_SIZE];
		int number;
		char *color[10];
		}player;


	// Global Variables ------------------------------------------------------------
	HANDLE P_Mutex;
	HANDLE gameStart;
	int p_count;
	player p1, p2;
	HANDLE ThreadHandles[NUM_OF_WORKER_THREADS];
	SOCKET ThreadInputs[NUM_OF_WORKER_THREADS];
	unsigned short ServerPort;
	int gameBoard[BOARD_HEIGHT][BOARD_WIDTH];
	FILE *fp_server_log;

	// Function Declarations -------------------------------------------------------
	void MainServer(char *argv[]);
	static int FindFirstUnusedThreadSlot();
	static void CleanupWorkerThreads();
	static DWORD ServiceThread(SOCKET *t_socket);
	void ServerMSG(int msgType, char *msgStr, SOCKET t_socket);
	void printServerLog(char *msg, BOOL closeFile);
	void clear_player(player *thrdPlayer);

	// String handlers
	int parseMessage(char *in_str, char *out_str);
	char* removeCharacter(char* str, char find);
	char* insertSemicolon(char* str);
	char* replace_char(char* str, char find, char replace);
	char *trimwhitespace(char *str);

#endif // SOCKET_EXAMPLE_SERVER_H