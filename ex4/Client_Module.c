/*
Authors			- Eli Slavutsky (308882992) & Nisan Tagar (302344031)
Project Name	- Ex4
Description		-
*/

// Includes --------------------------------------------------------------------
#include "Server_Module.h"
#include "Client_Module.h"
#include "SocketSendRecvTools.h"




//Reading data coming from the server
/*
Function 
------------------------
Description – 
Parameters	– 
Returns		– 
*/
static DWORD RecvDataThread(void)
{
	TransferResult_t RecvRes;

	while (!game_ended)
	{
		
		RecvRes = ReceiveString( &server_to_client, m_socket );

		if ( RecvRes == TRNS_FAILED )
		{
			printf("Socket error while trying to write data to socket\n");
			game_ended = 1;
			return 0x555;
		}
		else if ( RecvRes == TRNS_DISCONNECTED )
		{
			printf("Server closed connection. Bye!\n");
			game_ended = 1;
			return 0x555;
		}
		else
		{
			// need to add printing to log file - "Recieved from server: <raw message>
			cmd_to_action(server_to_client);
		}
		

	}

	return 0;
}


//Sending data to the server
/*
Function
------------------------
Description –
Parameters	–
Returns		–
*/
static DWORD SendDataThread(void)
{

	TransferResult_t SendRes;

	while (!game_ended)
	{
		if (cmd_ready)
		{
			if (STRINGS_ARE_EQUAL(cmd_to_server, "quit")) 
			{
				game_ended = 1;
				return 0x555; //"quit" signals an exit from the client side
			}

			
			SendRes = SendString(cmd_to_server, m_socket);
			cmd_ready = 0;

			if (SendRes == TRNS_FAILED)
			{
				printf("Socket error while trying to write data to socket\n");
				game_ended = 1;
				return 0x555;
			}
			else {
				// need to add printing to log file "Sent to Server: <raw message>"
			};
		}
		else		// new command is not ready yet
			continue;
	}
}


//Sending data to the server
/*
Function
------------------------
Description –
Parameters	–
Returns		–
*/
static DWORD player_input(void)
{
	char input		[MAX_MSG_SIZE];

	printf("Enter User Name\n");

	while (!game_ended)
	{
		gets_s(input, sizeof(input)); //Reading a string from the keyboard

		if (STRINGS_ARE_EQUAL(input, "exit"))
		{
			game_ended = 1;
			return 0x555; //"quit" signals an exit from the client side
		}

		input_to_cmd(input, cmd_to_server);
		cmd_ready = 1;			// update the sending thread that there is a new message ready
	}
}

/*
Function
------------------------
Description –
Parameters	–
Returns		–
*/
void MainClient(int argc, char *argv[])
{
	SOCKADDR_IN clientService;
	HANDLE hThread[3];

    // Initialize Winsock.
    WSADATA wsaData; //Create a WSADATA object called wsaData.
	//The WSADATA structure contains information about the Windows Sockets implementation.
	
	//Call WSAStartup and check for errors.
    int iResult = WSAStartup( MAKEWORD(2, 2), &wsaData );
    if ( iResult != NO_ERROR )
        printf("Error at WSAStartup()\n");

	//Call the socket function and return its value to the m_socket variable. 
	// For this application, use the Internet address family, streaming sockets, and the TCP/IP protocol.
	
	// Create a socket.
    m_socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

	// Check for errors to ensure that the socket is a valid socket.
    if ( m_socket == INVALID_SOCKET ) {
        printf( "Error at socket(): %ld\n", WSAGetLastError() );
        WSACleanup();
        return;
    }
	/*
	 The parameters passed to the socket function can be changed for different implementations. 
	 Error detection is a key part of successful networking code. 
	 If the socket call fails, it returns INVALID_SOCKET. 
	 The if statement in the previous code is used to catch any errors that may have occurred while creating 
	 the socket. WSAGetLastError returns an error number associated with the last error that occurred.
	 */


	//For a client to communicate on a network, it must connect to a server.
    // Connect to a server.

    //Create a sockaddr_in object clientService and set  values.
    clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr( SERVER_ADDRESS_STR ); //Setting the IP address to connect to
    //clientService.sin_port = htons( SERVER_PORT ); //Setting the port to connect to.
	
	/*
		AF_INET is the Internet address family. 
	*/


    // Call the connect function, passing the created socket and the sockaddr_in structure as parameters. 
	// Check for general errors.
	if ( connect( m_socket, (SOCKADDR*) &clientService, sizeof(clientService) ) == SOCKET_ERROR) {
        printf( "Failed to connect.\n" );
		// add printing to log file
        WSACleanup();
        return;
    }

    // Send and receive data.
	/*
		In this code, two integers are used to keep track of the number of bytes that are sent and received. 
		The send and recv functions both return an integer value of the number of bytes sent or received, 
		respectively, or an error. Each function also takes the same parameters: 
		the active socket, a char buffer, the number of bytes to send or receive, and any flags to use.

	*/	

	hThread[0]=CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE) SendDataThread,
		NULL,
		0,
		NULL
	);
	hThread[1]=CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE) RecvDataThread,
		NULL,
		0,
		NULL
	);
	hThread[2] = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)player_input,
		NULL,
		0,
		NULL
	);


	WaitForMultipleObjects(3,hThread,FALSE,INFINITE);

	TerminateThread(hThread[0],0x555);
	TerminateThread(hThread[1],0x555);
	TerminateThread(hThread[2],0x555);

	CloseHandle(hThread[0]);
	CloseHandle(hThread[1]);
	CloseHandle(hThread[2]);

	closesocket(m_socket);
	
	WSACleanup();
    
	return;
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
		msg_ptr = replace_char(msg_ptr, ';', ' ');
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
char* replace_char(char* str, char find, char replace) {
	char *current_pos = strchr(str, find);
	while (current_pos) {
		*current_pos = replace;
		current_pos = strchr(current_pos, find);
	}
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


/*
Function
------------------------
Description –
Parameters	–
Returns		–
*/
int getMsgStr(char *in_str, char *out_str) {

}

