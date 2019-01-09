/*
Authors			- Eli Slavutsky (308882992) & Nisan Tagar (302344031)
Project Name	- Ex4
Description		-
*/

// Includes --------------------------------------------------------------------

#include "Server_Module.h"
#include "SocketSendRecvTools.h"


// Function Definitions --------------------------------------------------------
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
	FILE *fp_server_log = fopen(argv[2], "wt");
	if (fp_server_log == NULL) {
		printf("ERROR: Failed to open server log file stream, can't complete the task! \n");
		goto file_stream_fail;
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

    printf( "Waiting for a client to connect...\n" );
    

	while (1) // Change to while on some endGame flag..
	{
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

file_stream_fail:
	return;
}


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
static DWORD ServiceThread( SOCKET *t_socket ) 
{
	char SendStr[SEND_STR_SIZE];
	BOOL endConnect = FALSE;
	TransferResult_t SendRes;
	TransferResult_t RecvRes;


	strcpy( SendStr, "Welcome to this server!" );
	SendRes = SendString( SendStr, *t_socket );
	if ( SendRes == TRNS_FAILED ) 
	{
		printf( "Service socket error while writing, closing thread.\n" );
		closesocket( *t_socket );
		return 1;
	}
	
	while ( !endConnect)
	{		
		char *AcceptedStr = NULL;
		
		RecvRes = ReceiveString( &AcceptedStr , *t_socket );

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

		free( AcceptedStr );		
	}

	printf("Conversation ended.\n");
	closesocket( *t_socket );
	return 0;
}


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
	}

	strcpy(out_str, msg_ptr);
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


