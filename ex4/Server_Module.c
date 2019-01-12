/*
Authors			- Eli Slavutsky (308882992) & Nisan Tagar (302344031)
Project Name	- Ex4
Description		-
*/

// Includes --------------------------------------------------------------------

#include "Server_Module.h"
#include "SocketSendRecvTools.h"

// Global Definitions ----------------------------------------------------------
p_count = 0;

// Function Definitions --------------------------------------------------------

/*
Function
------------------------
Description –
Parameters	–
Returns		–
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
		NULL,   /* default security attributes */
		FALSE,	/* don't lock mutex immediately */
		NULL); /* un-named */
	if (P_Mutex == NULL) {
		printf("Encountered error while creating mutex, ending program. Last Error = 0x%x\n", GetLastError());
		goto server_cleanup_0;
	}
	// Create event 
	gameStart = CreateEvent(
		NULL, /* default security attributes */
		TRUE,       /* manual-reset event */
		FALSE,      /* initial state is non-signaled */
		NULL);         /* name */
	if (gameStart == NULL) {
		printf("Encountered error while creating event, ending program. Last Error = 0x%x\n", GetLastError());
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
    service.sin_port = htons( ServerPort ); //The htons function converts a u_short from host to TCP/IP network byte order 
	                                   //( which is big-endian ).
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
	init_players_newGame();

	
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
Function
------------------------
Description –
Parameters	–
Returns		–
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
Function
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
	int msgType;
	player *thrdPlayer = NULL;
	BOOL endConnect = FALSE;
	BOOL playerAsigned = FALSE;
	TransferResult_t SendRes;
	TransferResult_t RecvRes;
	DWORD Res;
	
	// Assign player to handling thread and accept player
	while (!playerAsigned || p_count != 2) {
		RecvRes = ReceiveString(AcceptedStr, *t_socket);
		if (RecvRes == TRNS_FAILED)
		{
			printf("Service socket error while reading, closing thread.\n");
			printServerLog("Custom message: Service socket error while reading, closing thread.\n", TRUE);
			closesocket(*t_socket);
			exit (ERROR_CODE);
		}
		else if (RecvRes == TRNS_DISCONNECTED)
		{
			printf("Player disconnected. Ending communication.\n");
			printServerLog("Player disconnected. Ending communication.\n", false);
			closesocket(*t_socket);
			clear_player(thrdPlayer);
			return (SUCCESS_CODE);
		}

		msgType = parseMessage(AcceptedStr, paramStr);

		if (msgType != NEW_USER_REQUEST) {
			ServerMSG(PLAY_DECLINED, insertSemicolon("Game has not started"), *t_socket);
			continue;
		}

		// Handle new user request
		else if(!playerAsigned){
			if (asignThrdPlayer(paramStr, &thrdPlayer, *t_socket) != 0){

				continue;	// NEW_USER_DECLINED
			}
			else {
				printf("New %s player accepted to the game!\n", thrdPlayer->color);
				playerAsigned = TRUE; // NEW_USER_ACCEPTED
			}
		}
	}
	
	// Wait for 2 players to be accepted
	/*
	Res = WaitForSingleObject(gameStart, INFINITE);
	if (Res != WAIT_OBJECT_0) {
		printf("Error when waiting for game start event\n");
		printServerLog("Custom message: Error when waiting for game start event\n", TRUE);
		exit(ERROR_CODE);
	}
	*/


	//Game started, Turn Switch, Board View
	ServerMSG(GAME_STARTED, NULL, *t_socket);


	while ( !endConnect)
	{		
		RecvRes = ReceiveString( AcceptedStr , *t_socket );

		if ( RecvRes == TRNS_FAILED )
		{
			printf( "Service socket error while reading, closing thread.\n" );
			closesocket( *t_socket );
			return 1;
		}
		else if ( RecvRes == TRNS_DISCONNECTED )
		{
			printf( "Connection closed while reading, closing thread.\n" );
			closesocket( *t_socket );
			return 1;
		}
		else
		{
			printf( "Got string : %s\n", AcceptedStr );
		}

		//After reading a single line, checking to see what to do with it
		
		
		if ( STRINGS_ARE_EQUAL( AcceptedStr , "hello" ) ) 
			{ strcpy( SendStr, "what's up?" );} 
		else if ( STRINGS_ARE_EQUAL( AcceptedStr , "how are you?" ) ) 
			{ strcpy( SendStr, "great" ); }
		else if ( STRINGS_ARE_EQUAL( AcceptedStr, "bye" )) 
		{
			strcpy( SendStr, "see ya!" );
			endConnect = TRUE;
		}
		else 
			{ strcpy( SendStr, "I don't understand" ); }

		SendRes = SendString( SendStr, *t_socket );
	
		if ( SendRes == TRNS_FAILED ) 
		{
			printf( "Service socket error while writing, closing thread.\n" );
			closesocket( *t_socket );
			return 1;
		}
	
	}

	printf("Conversation ended.\n");
	closesocket( *t_socket );
	return 0;
}


// Player routines

void clear_player(player *thrdPlayer) {
	if (thrdPlayer == NULL) {
		return;
	}
	else {
		strcpy(thrdPlayer->name, "");
		thrdPlayer->myTurn = FALSE;
		thrdPlayer->accepted = FALSE;
		p_count--;
	}
	return;
}



/*
Function
------------------------
Description –
Parameters	–
Returns		–
*/
int init_players_newGame() {
		p1.accepted = FALSE;
		strcpy(p1.name, "");
		p1.number = RED_PLAYER;
		p1.myTurn = TRUE;
		strcpy(p1.color, "Red");

		p2.accepted = FALSE;
		strcpy(p2.name, "");
		p2.number = YELLOW_PLAYER;
		p2.myTurn = FALSE;
		strcpy(p2.color, "Yellow");
	return 0;
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
	char num[2];
	char SendStr[MAX_MSG_SIZE];
	bool is_success;

	// Wait for player mutex
	wait_res = WaitForSingleObject(P_Mutex, INFINITE);
	if (wait_res != WAIT_OBJECT_0) {
		printf("Error when waiting for player mutex\n");
		printServerLog("Custom message: Error when waiting for player mutex\n", TRUE);
		exit(ERROR_CODE);
	}

	if (p1.accepted == FALSE) { 
		// p1 is available
		if (STRINGS_ARE_EQUAL(name, p2.name)) {
			ret = -1;
		}
		else {
			p1.accepted = TRUE;
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
			p2.accepted = TRUE;
			strcpy(p2.name, name);
			p_count++;
			*p = &p2;
		}
	}
	// Check if 2 players are present
	if (p_count == 2) {
		is_success = SetEvent(gameStart);
		if (is_success == FALSE) {
			printf("Error when settings game start event\n");
			printServerLog("Custom message: Error when settings game start event\n", TRUE);
			exit(ERROR_CODE);
		}
	}

	release_res = ReleaseMutex(P_Mutex);
	if (release_res == FALSE) {
		printf("Error when waiting for player mutex\n");
		printServerLog("Custom message: Error when waiting for player mutex\n", TRUE);
		exit(ERROR_CODE);
	}

	// Send msg to client
	if (ret == -1) {
		ServerMSG(NEW_USER_DECLINED, NULL, t_socket);
	}
	else if (ret == 0) {
		itoa(p_count, num,10);
		ServerMSG(NEW_USER_ACCEPTED, num, t_socket);
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
		break;
	case TURN_SWITCH:
		/*
		if (p1.myTurn) {
			strcpy(SendStr, "TURN_SWITCH:");
			strcat(SendStr, p1.name);
		}
		else if (p2.myTurn){
			strcpy(SendStr, "TURN_SWITCH:");
			strcat(SendStr, p2.name);
		}
		*/
		strcpy(SendStr, "TURN_SWITCH:");
		strcat(SendStr, msgStr);
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
		strcpy(SendStr, "GAME_ENDED");
		strcat(SendStr, msgStr);
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
		printServerLog("Custom message: Service socket error while writing.", TRUE);
		exit(ERROR_CODE);
	}
}


/*
Function:
------------------------
Description – The function receive pointer to a string and trimms the string from white spaces
Parameters	– *str is a pointer to a string to be trimmed.
Returns		– Retrun pointer to trimmed string
*/
void printServerLog(char *msg, BOOL closeFile) {
	fprintf(fp_server_log, "%s", msg);
	if (closeFile) {
		fclose(fp_server_log);
	}
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
