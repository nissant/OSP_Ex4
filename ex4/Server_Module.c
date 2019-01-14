/*
Authors			- Eli Slavutsky (308882992) & Nisan Tagar (302344031)
Project Name	- Ex4
Description		- Connect 4 - A Server/Client game with human or file mode players
				- Server module handles player connection management and game flow control
*/


// Includes --------------------------------------------------------------------
#include "Server_Module.h"
#include "SocketSendRecvTools.h"

// Global Definitions ----------------------------------------------------------

// Function Definitions --------------------------------------------------------

/*
Function MainServer
------------------------
Description – Main Server routine. Handlese win socket init, game init, client connections and thread management/recycling
Parameters	– *argv[] - command line arguments, it is provided that there are sufficient arguments for program requirements
Returns		– none
*/
void MainServer(char *argv[])
{
	int Ind;
	int Loop;
	SOCKET MainSocket = INVALID_SOCKET;
	unsigned long Address;
	SOCKADDR_IN service;
	int bindRes;
	int ListenRes;

	ServerPort = (unsigned short)strtol(argv[3], NULL, 10);
	fp_server_log = fopen(argv[2], "wt");
	if (fp_server_log == NULL) {
		printf("ERROR: Failed to open server log file stream, can't complete the task! \n");
		goto server_cleanup_0;
	}

	// Create player mutex
	P_Mutex = CreateMutex(
		NULL,		/* default security attributes */
		false,		/* don't lock mutex immediately */
		NULL);		/* un-named */
	if (P_Mutex == NULL) {
		printf("Encountered error while creating players mutex, ending program. Last Error = 0x%x\n", GetLastError());
		goto server_cleanup_0;
	}

	// Create board game mutex
	board_Mutex = CreateMutex(
		NULL,		/* default security attributes */
		false,		/* don't lock mutex immediately */
		NULL);		/* un-named */
	if (board_Mutex == NULL) {
		printf("Encountered error while creating board game mutex, ending program. Last Error = 0x%x\n", GetLastError());
		goto server_cleanup_0;
	}

	// Initialize Winsock.
    WSADATA wsaData;
    int StartupRes = WSAStartup( MAKEWORD( 2, 2 ), &wsaData );	           

    if ( StartupRes != NO_ERROR )
	{
        printf( "error %ld at WSAStartup( ), ending program.\n", WSAGetLastError() );
		// Tell the user that we could not find a usable WinSock DLL.                                  
		return;
	}
 
    /* The WinSock DLL is acceptable. Proceed. */

    // Create a socket.    
    MainSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

    if ( MainSocket == INVALID_SOCKET ) 
	{
        printf( "Error at socket( ): %ld\n", WSAGetLastError( ) );
		goto server_cleanup_1;
    }

    // Bind the socket.
	/*
		For a server to accept client connections, it must be bound to a network address within the system. 
		The following code demonstrates how to bind a socket that has already been created to an IP address 
		and port.
		Client applications use the IP address and port to connect to the host network.
		The sockaddr structure holds information regarding the address family, IP address, and port number. 
		sockaddr_in is a subset of sockaddr and is used for IP version 4 applications.
   */
	// Create a sockaddr_in object and set its values.
	// Declare variables

	Address = inet_addr( SERVER_ADDRESS_STR );
	if ( Address == INADDR_NONE )
	{
		printf("The string \"%s\" cannot be converted into an ip address. ending program.\n",
				SERVER_ADDRESS_STR );
		goto server_cleanup_2;
	}

    service.sin_family = AF_INET;
    service.sin_addr.s_addr = Address;
    service.sin_port = htons( ServerPort ); //The htons function converts a u_short from host to TCP/IP network byte order ( which is big-endian ).

	/*
		The three lines following the declaration of sockaddr_in service are used to set up 
		the sockaddr structure: 
		AF_INET is the Internet address family. 
		"127.0.0.1" is the local IP address to which the socket will be bound. 
	    2345 is the port number to which the socket will be bound.
	*/

	// Call the bind function, passing the created socket and the sockaddr_in structure as parameters. 
	// Check for general errors.
    bindRes = bind( MainSocket, ( SOCKADDR* ) &service, sizeof( service ) );
	if ( bindRes == SOCKET_ERROR ) 
	{
        printf( "bind( ) failed with error %ld. Ending program\n", WSAGetLastError( ) );
		goto server_cleanup_2;
	}
    
    // Listen on the Socket.
	ListenRes = listen( MainSocket, SOMAXCONN );
    if ( ListenRes == SOCKET_ERROR ) 
	{
        printf( "Failed listening on socket, error %ld.\n", WSAGetLastError() );
		goto server_cleanup_2;
	}

	// Initialize all thread handles to NULL, to mark that they have not been initialized
	for ( Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++ )
		ThreadHandles[Ind] = NULL;
    
	// Initialize players variables
	init_newGame();

	while (1)
	{
		printf("Waiting for a client player to connect...\n");
		SOCKET AcceptSocket = accept( MainSocket, NULL, NULL );
		if ( AcceptSocket == INVALID_SOCKET )
		{
			printf( "Accepting connection with client failed, error %ld\n", WSAGetLastError() ) ; 
			goto server_cleanup_3;
		}

        printf( "Client Connected.\n" );

		Ind = FindFirstUnusedThreadSlot();

		if ( Ind == NUM_OF_WORKER_THREADS ) //no slot is available
		{ 
			printf( "No slots available for client, dropping the connection.\n" );
			closesocket( AcceptSocket ); //Closing the socket, dropping the connection.
		} 
		else 	
		{
			ThreadInputs[Ind] = AcceptSocket; // shallow copy: don't close 
											  // AcceptSocket, instead close 
											  // ThreadInputs[Ind] when the
											  // time comes.
			ThreadHandles[Ind] = CreateThread(
				NULL,
				0,
				( LPTHREAD_START_ROUTINE ) ServiceThread,
				&( ThreadInputs[Ind] ),
				0,
				NULL
			);
		}
    } 

server_cleanup_3:

	CleanupWorkerThreads();

server_cleanup_2:
	if ( closesocket( MainSocket ) == SOCKET_ERROR )
		printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError() ); 

server_cleanup_1:
	if ( WSACleanup() == SOCKET_ERROR )		
		printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError() );

	fclose(fp_server_log);

server_cleanup_0:
	return;
}


/*
Function FindFirstUnusedThreadSlot
------------------------
Description – Thread handle recycling routine, makes sure that closed threads handles are closed and reset to null after use
Parameters	– ThreadHandles[Ind] global handle array
Returns		– none
*/
static int FindFirstUnusedThreadSlot()
{ 
	int Ind;

	for ( Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++ )
	{
		if ( ThreadHandles[Ind] == NULL )
			break;
		else
		{
			// poll to check if thread finished running:
			DWORD Res = WaitForSingleObject( ThreadHandles[Ind], 0 ); 
				
			if ( Res == WAIT_OBJECT_0 ) // this thread finished running
			{				
				CloseHandle( ThreadHandles[Ind] );
				ThreadHandles[Ind] = NULL;
				break;
			}
		}
	}

	return Ind;
}


/*
Function CleanupWorkerThreads
------------------------
Description – 
Parameters	–
Returns		–
*/
static void CleanupWorkerThreads()
{
	int Ind; 

	for ( Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++ )
	{
		if ( ThreadHandles[Ind] != NULL )
		{
			// poll to check if thread finished running:
			DWORD Res = WaitForSingleObject( ThreadHandles[Ind], INFINITE ); 
				
			if ( Res == WAIT_OBJECT_0 ) 
			{
				closesocket( ThreadInputs[Ind] );
				CloseHandle( ThreadHandles[Ind] );
				ThreadHandles[Ind] = NULL;
				break;
			}
			else
			{
				printf( "Waiting for thread failed. Ending program\n" );
				return;
			}
		}
	}
}


//Service thread is the thread that opens for each successful client connection and "talks" to the client.
/*
Function
------------------------
Description –
Parameters	–
Returns		–
*/
static DWORD ServiceThread(SOCKET *t_socket) 
{
	char SendStr[MAX_MSG_SIZE];
	char AcceptedStr[MAX_MSG_SIZE];
	char paramStr[MAX_MSG_SIZE];
	char num[5];
	int msgType;
	player *thrdPlayer = NULL;
	bool playerAsigned = false;
	TransferResult_t SendRes;
	TransferResult_t RecvRes;
	DWORD Res;
	
	// Assign player to handling thread and accept player
	while (!playerAsigned) {
		RecvRes = ReceiveString(AcceptedStr, *t_socket);
		if (RecvRes == TRNS_FAILED)
		{
			printf("Service socket error while reading, closing thread.\n");
			printServerLog("Custom message: Service socket error while reading, closing thread.\n", true);
			init_newGame();
			return SUCCESS_CODE;
		}
		else if (RecvRes == TRNS_DISCONNECTED)
		{
			printf("Player disconnected. Ending communication.\n");
			printServerLog("Player disconnected. Ending communication.\n", false);
			init_newGame();
			return SUCCESS_CODE;
		}
		// Parse message
		msgType = parseMessage(AcceptedStr, paramStr);
		if (msgType != NEW_USER_REQUEST) {
			continue;
		}
		// Handle new user request
		if (asignThrdPlayer(paramStr, &thrdPlayer, *t_socket) != 0){
			// NEW_USER_DECLINED
			ServerMSG(NEW_USER_DECLINED, NULL, *t_socket);
			continue;	
		}
		else {
			// NEW_USER_ACCEPTED
			itoa(p_count, num, 10);
			ServerMSG(NEW_USER_ACCEPTED, num, *t_socket);
			thrdPlayer->S = *t_socket;		// A player will be diconnected only after being accepted to game but not while trying to register
			printf("New %s player accepted to the game!\n", thrdPlayer->color);
			playerAsigned = true; // NEW_USER_ACCEPTED
		}
	}

	// At this piont, a user is assigned to this thread and has a specific variable (p1,p2)
	HANDLE player_Thrds[2];
	// Open Helper Threads 
	player_Thrds[0] = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)Server_Rec_Thread,
		thrdPlayer,
		0,
		NULL
	);

	player_Thrds[1] = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)Server_Send_Thread,
		thrdPlayer,
		0,
		NULL
	);

	Res = WaitForMultipleObjects(2, player_Thrds, true, INFINITE);
	if (Res != WAIT_OBJECT_0)
	{
		printf("Service socket error while waiting for message thread, closing thread.\n");
		printServerLog("Custom message: Service socket error while waiting for message thread, closing thread.", true);
		closesocket(*t_socket);
		exit(ERROR_CODE);
	}
	CloseHandle(player_Thrds[0]);
	CloseHandle(player_Thrds[1]);
	init_newGame();
	return SUCCESS_CODE;
}

static DWORD Server_Send_Thread(player *thrdPlayer) {
	char paramStr[MAX_MSG_SIZE];
	bool gameStarted = true;
	bool turnFlag = thrdPlayer->myTurn;
	bool keepWorking = true;

	while (keepWorking && thrdPlayer->playing) {
		if (p_count == 2 && gameStarted) {
			//Game started, Turn Switch, Board View
			ServerMSG(GAME_STARTED, NULL, thrdPlayer->S);
			ServerMSG(TURN_SWITCH, NULL, thrdPlayer->S);
			ServerMSG(BOARD_VIEW, itoa(MAXINT, paramStr, 10), thrdPlayer->S);	// Signal print empty board on clients
			gameStarted = false;
		}
		
		if (turnFlag != thrdPlayer->myTurn) {									// Turns have changed
			ServerMSG(TURN_SWITCH, boardUpdate, thrdPlayer->S);
			ServerMSG(BOARD_VIEW, boardUpdate, thrdPlayer->S);
			turnFlag = thrdPlayer->myTurn;
		}

		if (thrdPlayer->result != -1) {
			// Results are in
			ServerMSG(GAME_ENDED, NULL, thrdPlayer->S);
			keepWorking = false;
		}

		check_incoming_msg(thrdPlayer, thrdPlayer->S);							// Check for incoming message, if true send RECEIVE_MESSAGE
	}
	return (SUCCESS_CODE);
}

static DWORD Server_Rec_Thread(player *thrdPlayer) {
	char AcceptedStr[MAX_MSG_SIZE];
	char paramStr[MAX_MSG_SIZE];
	bool endGame = false;
	TransferResult_t SendRes;
	TransferResult_t RecvRes;
	int msgType;
	
	// Play the game
	while (thrdPlayer->playing)
	{
		RecvRes = ReceiveString(AcceptedStr, thrdPlayer->S);
		if (RecvRes == TRNS_FAILED)
		{
			printf("Service socket error while reading, closing thread.\n");
			printServerLog("Custom message: Service socket error while reading, closing thread.\n", false);
			thrdPlayer->playing = false;	// Signal thrd
			return SUCCESS_CODE;
		}
		else if (RecvRes == TRNS_DISCONNECTED)
		{
			printf("Player disconnected. Ending communication.\n");
			printServerLog("Player disconnected. Ending communication.\n", false);
			thrdPlayer->playing = false;	// Signal thrd
			return SUCCESS_CODE;
		}
		if (p_count != 2) {
			strcpy(paramStr, "Game has not started");
			ServerMSG(PLAY_DECLINED, paramStr, thrdPlayer->S);
			continue;
		}
		
		// Parse message
		msgType = parseMessage(AcceptedStr, paramStr);
		if (msgType == PLAY_REQUEST) {
			// Handle player move
			int row, col;
			if (handle_move(paramStr, thrdPlayer, &row, &col)) {		// Check move & send accept/decline, update server board and send board view
				verdict_or_switch(thrdPlayer, row, col);				// Check if ther is a game final result, if true send GAME_ENDED to each player
			}
			else {
				// Play not legitimate
				continue;
			}
		}
		else if (msgType == SEND_MESSAGE) {
			// Handle player outging message
			send_outgoing_msg(paramStr, thrdPlayer, thrdPlayer->S);	// Send message internally to other player
		}
	}
	return SUCCESS_CODE;
}

// Player routines

/*
Function
------------------------
Description –
Parameters	–
Returns		–
*/
int init_newGame() {
	int i, j;

	p1.playing = false;
	strcpy(p1.name, "");
	p1.number = RED_PLAYER;
	p1.myTurn = true;
	p1.gotMessage = false;
	strcpy(p1.color, "Red");
	p1.result = -1;
	shutdown(p1.S, SD_BOTH);

	p2.playing = false;
	strcpy(p2.name, "");
	p2.number = YELLOW_PLAYER;
	p2.myTurn = false;
	p2.gotMessage = false;
	strcpy(p2.color, "Yellow");
	p2.result = -1;
	shutdown(p2.S, SD_BOTH);

	p_count = 0;
	init_server_board();

	return 0;
}


/*
Function
------------------------
Description –
Parameters	–
Returns		–
*/
void init_server_board()
{
	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j < 7; j++)
		{
			serverBoard[i][j] = 0;
		}
	}
}


/*
Function
------------------------
Description –
Parameters	–
Returns		–
*/
int asignThrdPlayer(char *name, player **p, SOCKET t_socket) {
	DWORD wait_res, release_res;
	TransferResult_t SendRes;
	int ret = 0;
	char SendStr[MAX_MSG_SIZE];
	bool is_success;

	// Wait for player mutex
	wait_res = WaitForSingleObject(P_Mutex, INFINITE);
	if (wait_res != WAIT_OBJECT_0) {
		printf("Error when waiting for player mutex\n");
		printServerLog("Custom message: Error when waiting for player mutex\n", true);
		exit(ERROR_CODE);
	}

	if (p1.playing == false) { 
		// p1 is available
		if (STRINGS_ARE_EQUAL(name, p2.name)) {
			ret = -1;
		}
		else {
			p1.playing = true;
			strcpy(p1.name, name);
			p_count++;
			*p = &p1;
		}
	}
	else {						
		// p2 is available
		if (STRINGS_ARE_EQUAL(name, p1.name)) {
			ret = -1;
		}
		else {
			p2.playing = true;
			strcpy(p2.name, name);
			p_count++;
			*p = &p2;
		}
	}

	release_res = ReleaseMutex(P_Mutex);
	if (release_res == false) {
		printf("Error when waiting for player mutex\n");
		printServerLog("Custom message: Error when waiting for player mutex\n", true);
		exit(ERROR_CODE);
	}
	return ret;
}


/*
Function
------------------------
Description –
Parameters	–
Returns		–
*/
void ServerMSG(int msgType, char *msgStr, SOCKET t_socket) {
	TransferResult_t SendRes;
	char SendStr[MAX_MSG_SIZE];
	insertSemicolon(msgStr);
	switch (msgType) {
	case NEW_USER_ACCEPTED:
		strcpy(SendStr, "NEW_USER_ACCEPTED:");
		strcat(SendStr, msgStr);
		SendRes = SendString(SendStr, t_socket);
		break;
	case NEW_USER_DECLINED:
		SendRes = SendString("NEW_USER_DECLINED", t_socket);
		break;
	case GAME_STARTED:
		SendRes = SendString("GAME_STARTED", t_socket);
		break;
	case BOARD_VIEW:
		strcpy(SendStr, "BOARD_VIEW:");
		strcat(SendStr, msgStr);
		SendRes = SendString(SendStr, t_socket);
		break;
	case TURN_SWITCH:
		strcpy(SendStr, "TURN_SWITCH:");
		if (p1.myTurn) {
			strcat(SendStr, p1.name);
		}
		else if (p2.myTurn){ // Sanity
			strcat(SendStr, p2.name);
		}
		SendRes = SendString(SendStr, t_socket);
		break;
	case PLAY_ACCEPTED:
		SendRes = SendString("PLAY_ACCEPTED", t_socket);
		break;
	case PLAY_DECLINED:
		strcpy(SendStr, "PLAY_DECLINED:");
		strcat(SendStr, msgStr);
		SendRes = SendString(SendStr, t_socket);
		break;
	case GAME_ENDED:
		
		if (p1.result == WINNER) {
			strcpy(SendStr, "GAME_ENDED:");
			strcat(SendStr, p1.name);
		}
		else if (p2.result == WINNER) {
			strcpy(SendStr, "GAME_ENDED:");
			strcat(SendStr, p2.name);
		}
		else if (p2.result == DRAW && p1.result == DRAW) {	// Sanity test
			strcpy(SendStr, "GAME_ENDED");
		}
		SendRes = SendString(SendStr, t_socket);
		break;
	case RECEIVE_MESSAGE:
		strcpy(SendStr, "RECEIVE_MESSAGE:");
		strcat(SendStr, msgStr);
		SendRes = SendString(SendStr, t_socket);
		break;
	default:
		SendRes = SendString(SendStr, t_socket);
	}
	
	if (SendRes == TRNS_FAILED)
	{
		printf("Service socket error while writing.\n");
		closesocket(t_socket);
		printServerLog("Custom message: Service socket error while writing.", true);
		exit(ERROR_CODE);
	}
}


/*
Function
------------------------
Description –
Parameters	–
Returns		–
*/
void check_incoming_msg(player *thrdPlayer, SOCKET t_socket) {	
	DWORD wait_res, release_res;
	char tmpStr[MAX_MSG_SIZE];

	if (thrdPlayer->gotMessage == false) {
		return;
	}

	// Make sure Wait for player mutex
	wait_res = WaitForSingleObject(P_Mutex, INFINITE);
	if (wait_res != WAIT_OBJECT_0) {
		printf("Error when waiting for player mutex\n");
		printServerLog("Custom message: Error when waiting for player mutex\n", true);
		exit(ERROR_CODE);
	}

	strcpy(tmpStr, thrdPlayer->msg);
	thrdPlayer->gotMessage = false;
	strcpy(thrdPlayer->msg,"");

	release_res = ReleaseMutex(P_Mutex);
	if (release_res == false) {
		printf("Error when waiting for player mutex\n");
		printServerLog("Custom message: Error when waiting for player mutex\n", true);
		exit(ERROR_CODE);
	}

	// Send message
	ServerMSG(RECEIVE_MESSAGE, tmpStr, t_socket);
}


/*
Function
------------------------
Description –
Parameters	–
Returns		–
*/
bool handle_move(char *paramStr,player *thrdPlayer, int *row_p, int *col_p) {
	char tmpStr[MAX_MSG_SIZE];
	DWORD wait_res, release_res;
	// Sanity
	if (row_p == NULL || col_p == NULL) {
		printf("Wrong usage for function handle_move!\n");
		exit(ERROR_CODE);
	}
	// Check Turn
	if (thrdPlayer->myTurn == false) {
		strcpy(tmpStr, "Not your turn");
		ServerMSG(PLAY_DECLINED, tmpStr, thrdPlayer->S);
		return false;
	}

	// Parse string
	int col = (int)strtol(paramStr, NULL, 10);
	if (col > 6) {
		strcpy(tmpStr, "Illegal move");
		ServerMSG(PLAY_DECLINED, tmpStr, thrdPlayer->S);
		return false;
	}

	if (serverBoard[BOARD_HEIGHT-1][col] != 0) {
		strcpy(tmpStr, "Illegal move");
		ServerMSG(PLAY_DECLINED, tmpStr, thrdPlayer->S);
		return false;
	}

	// Wait for server board mutex
	wait_res = WaitForSingleObject(board_Mutex, INFINITE);
	if (wait_res != WAIT_OBJECT_0) {
		printf("Error when waiting for player mutex\n");
		printServerLog("Custom message: Error when waiting for player mutex\n", true);
		exit(ERROR_CODE);
	}
	int row;
	int i = BOARD_HEIGHT - 1;
	while (serverBoard[i][col] == 0 && i>=0) {
		i--;
	}
	row = i + 1;
	serverBoard[row][col] = thrdPlayer->number;
	
	release_res = ReleaseMutex(board_Mutex);
	if (release_res == false) {
		printf("Error when waiting for player mutex\n");
		printServerLog("Custom message: Error when waiting for player mutex\n", true);
		exit(ERROR_CODE);
	}

	// Pass arguments
	*col_p = col;
	*row_p = row;

	// Send PLAY_ACCEPTED
	char row_c[2];
	char coin[2];
	itoa(row, &row_c, 10);
	itoa(thrdPlayer->number, coin , 10);
	strcpy(boardUpdate, row_c);
	strcat(boardUpdate, paramStr);
	strcat(boardUpdate, coin);
	ServerMSG(PLAY_ACCEPTED, NULL, thrdPlayer->S);
	return true;
}



/*
Function
------------------------
Description –
Parameters	–
Returns		–
*/
void send_outgoing_msg(char *paramStr, player *thrdPlayer, SOCKET t_socket) {
	DWORD wait_res, release_res;
	char tmpStr[MAX_MSG_SIZE];

	// Find second player
	player *p = NULL;
	if (p1.number == thrdPlayer->number) {
		p = &p2;
	}
	else if (p2.number == thrdPlayer->number) {
		p = &p1;
	}

	if (p == NULL) {
		return;
	}

	// Make sure player is free
	wait_res = WaitForSingleObject(P_Mutex, INFINITE);
	if (wait_res != WAIT_OBJECT_0) {
		printf("Error when waiting for player mutex\n");
		printServerLog("Custom message: Error when waiting for player mutex\n", true);
		exit(ERROR_CODE);
	}

	strcpy(tmpStr, thrdPlayer->name);
	strcat(tmpStr, " ");
	strcat(tmpStr, paramStr);
	strcpy(p->msg, tmpStr);
	p->gotMessage = true;

	release_res = ReleaseMutex(P_Mutex);
	if (release_res == false) {
		printf("Error when waiting for player mutex\n");
		printServerLog("Custom message: Error when waiting for player mutex\n", true);
		exit(ERROR_CODE);
	}

}



/*
Function
------------------------
Description –
Parameters	–
Returns		–
*/
void verdict_or_switch(player *thrdPlayer,int row, int col) {
	int res = getResult(thrdPlayer->number, row, col);
	if (res == -1){
	// No results so switch turns
		p1.myTurn = !p1.myTurn;
		p2.myTurn = !p2.myTurn;
	}
	else {
		// Write results and raise end flags
		if (res == 0) {
			p1.result = DRAW;
			p2.result = DRAW;
		}
		else if (res == RED_PLAYER) {
			p1.result = WINNER;
			p2.result = LOOSER;
		}
		else if (res == YELLOW_PLAYER) {
			p1.result = LOOSER;
			p2.result = WINNER;
		}

		//thrdPlayer->playing = false;
	}
}


/*
Function:
------------------------
Description – The function receive pointer to a string and trimms the string from white spaces
Parameters	– *str is a pointer to a string to be trimmed.
Returns		– Retrun pointer to trimmed string
*/
void printServerLog(char *msg, bool closeFile) {
	fprintf(fp_server_log, "%s", msg);
	if (closeFile) {
		fclose(fp_server_log);
	}
}

// Connect 4 Routines

/*
Function:
------------------------
Description – The function receive pointer to a string and trimms the string from white spaces
Parameters	– *str is a pointer to a string to be trimmed.
Returns		–  -1 for no verdict, 0 for draw, 1 for red player won, 2 for yellow player won
*/
int getResult(int player, int row, int col) {
	if (areFourConnected(player, row, col)) {
		return player;
	}
	else if (isBoardFull()) {
		return 0;
	}
	else
		return -1;
}

/*
Function:
------------------------
Description – The function receive pointer to a string and trimms the string from white spaces
Parameters	– *str is a pointer to a string to be trimmed.
Returns		–  -1 for no verdict, 0 for draw, 1 for red player won, 2 for yellow player won
*/
bool areFourConnected(int player, int row, int col) {
		// horizontalCheck 
		for (int j = 0; j < BOARD_WIDTH - 3; j++) {
			if (serverBoard[row][j] == player && serverBoard[row][j + 1] == player && serverBoard[row][j + 2] == player && serverBoard[row][j + 3] == player) {
				return true;
			}
		}
		// verticalCheck
		for (int i = 0; i < BOARD_HEIGHT - 3; i++) {
			if (serverBoard[i][col] == player && serverBoard[i + 1][col] == player && serverBoard[i + 2][col] == player && serverBoard[i + 3][col] == player) {
				return true;
			}
		}
		// ascendingDiagonalCheck 
		for (int i = 3; i < BOARD_HEIGHT; i++) {
			for (int j = 0; j < BOARD_WIDTH - 3; j++) {
				if (serverBoard[i][j] == player && serverBoard[i - 1][j + 1] == player && serverBoard[i - 2][j + 2] == player && serverBoard[i - 3][j + 3] == player)
					return true;
			}
		}
		// descendingDiagonalCheck
		for (int i = 3; i < BOARD_HEIGHT; i++) {
			for (int j = 3; j < BOARD_WIDTH; j++) {
				if (serverBoard[i][j] == player && serverBoard[i - 1][j - 1] == player && serverBoard[i - 2][j - 2] == player && serverBoard[i - 3][j - 3] == player)
					return true;
			}
		}
		return false;
}


/*
Function:
------------------------
Description – The function receive pointer to a string and trimms the string from white spaces
Parameters	– *str is a pointer to a string to be trimmed.
Returns		–  -1 for no verdict, 0 for draw, 1 for red player won, 2 for yellow player won
*/

bool isBoardFull() {
	bool ret = true;
	for (int i = 0; i < BOARD_WIDTH; i++) {
		if (serverBoard[BOARD_HEIGHT - 1][i] == 0) {
			ret = false;
			return ret;
		}
	}
	return ret;
}



// String routines 

/*
Function
------------------------
Description –
Parameters	–
Returns		–
*/
int parseMessage(char *in_str, char *out_str) {
	int msg_type;
	char *msg_ptr;

	if (in_str == NULL || out_str == NULL) {
		return -1;
	}

	msg_ptr = strchr(in_str, ':');
	if (msg_ptr != NULL) {
		*msg_ptr = '\0';
		msg_ptr++;
		msg_ptr = removeCharacter(msg_ptr, ';');
		msg_ptr = trimwhitespace(msg_ptr);
		strcpy(out_str, msg_ptr);
	}

	
	in_str = trimwhitespace(in_str);

	// Look for message type string
	if (strstr(in_str, "NEW_USER_REQUEST") != NULL) {
		msg_type = NEW_USER_REQUEST;
	}
	else if (strstr(in_str, "NEW_USER_ACCEPTED") != NULL) {
		msg_type = NEW_USER_ACCEPTED;
	}
	else if (strstr(in_str, "NEW_USER_DECLINED") != NULL) {
		msg_type = NEW_USER_DECLINED;
	}
	else if (strstr(in_str, "GAME_STARTED") != NULL) {
		msg_type = GAME_STARTED;
	}
	else if (strstr(in_str, "BOARD_VIEW") != NULL) {
		msg_type = BOARD_VIEW;
	}
	else if (strstr(in_str, "TURN_SWITCH") != NULL) {
		msg_type = TURN_SWITCH;
	}
	else if (strstr(in_str, "PLAY_REQUEST") != NULL) {
		msg_type = PLAY_REQUEST;
	}
	else if (strstr(in_str, "PLAY_ACCEPTED") != NULL) {
		msg_type = PLAY_ACCEPTED;
	}
	else if (strstr(in_str, "PLAY_DECLINED") != NULL) {
		msg_type = PLAY_DECLINED;
	}
	else if (strstr(in_str, "GAME_ENDED") != NULL) {
		msg_type = GAME_ENDED;
	}
	else if (strstr(in_str, "SEND_MESSAGE") != NULL) {
		msg_type = SEND_MESSAGE;
	}
	else if (strstr(in_str, "RECEIVE_MESSAGE") != NULL) {
		msg_type = RECEIVE_MESSAGE;
	}
	else
		msg_type = BAD_MSG;

	return msg_type;
}


/*
Function
------------------------
Description –
Parameters	–
Returns
*/
char* removeCharacter(char* str, char find) {
	char temp[MAX_MSG_SIZE];
	char *ptr, *ptr2;

	if (str == NULL) {
		return NULL;
	}

	ptr = str;
	ptr2 = temp;
	while (*ptr != '\0') {
		if (*ptr != find) {
			*ptr2 = *ptr;
			ptr2++;
		}
		ptr++;
	}
	*ptr2 = '\0';
	strcpy(str, temp);
	return str;
}


/*
Function
------------------------
Description –
Parameters	–
Returns
*/
char* replace_char(char* str, char find, char replace) {
	char *current_pos = strchr(str, find);
	while (current_pos) {
		*current_pos = replace;
		current_pos = strchr(current_pos, find);
	}
	return str;
}


/*
Function
------------------------
Description –
Parameters	–
Returns
*/
char* insertSemicolon(char* str) {
	char temp[MAX_MSG_SIZE];
	char *ptr, *ptr2;

	if (str == NULL) {
		return NULL;
	}

	ptr = str;
	ptr2 = temp;
	while (*ptr != '\0') {
		if (*ptr == ' ') {
			*ptr2 = ';';
			ptr2++;
			*ptr2 = ' ';
			ptr2++;
			*ptr2 = ';';
			ptr2++;
		}
		else {
			*ptr2 = *ptr;
			ptr2++;
		}

		ptr++;
	}
	*ptr2 = '\0';
	strcpy(str, temp);
	return str;
}


/*
Function: trimwhitespace
------------------------
Description – The function receive pointer to a string and trimms the string from white spaces
Parameters	– *str is a pointer to a string to be trimmed.
Returns		– Retrun pointer to trimmed string
*/
char *trimwhitespace(char *str)
{
	char *end;

	// Trim leading space
	while (isspace((unsigned char)*str)) str++;

	if (*str == 0)  // All spaces?
		return str;

	// Trim trailing space
	end = str + strlen(str) - 1;
	while (end > str && isspace((unsigned char)*end)) end--;

	// Write new null terminator character
	end[1] = '\0';

	return str;
}
