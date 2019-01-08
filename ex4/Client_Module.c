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
Description � 
Parameters	� 
Returns		� 
*/
static DWORD RecvDataThread(void)
{
	while (1);
	/*
	TransferResult_t RecvRes;

	while (!game_ended)
	{
		
		RecvRes = ReceiveString( &server_to_client, m_socket );

		if ( RecvRes == TRNS_FAILED )
		{
			printf("Socket error while trying to read data from socket\n");
			fputs("Socket error while trying to read data from socket\n", client_log);
			game_ended = 1;
			exit(007);
		}
		else if ( RecvRes == TRNS_DISCONNECTED )
		{
			printf("Server disconnected. Exiting\n");
			fputs("Server disconnected. Exiting\n", client_log);
			game_ended = 1;
			return 0x555;
		}
		else
		{
			fputs(server_to_client, client_log);
			cmd_to_action(server_to_client);
		}
		

	}

	return 0;*/
}


//Sending data to the server
/*
Function
------------------------
Description �
Parameters	�
Returns		�
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
				exit(007);
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
Description �
Parameters	�
Returns		�
*/
static DWORD player_input(void)
{
	char input [MAX_MSG_SIZE];

	printf("Enter User Name\n");

	while (!game_ended)
	{

		gets_s(input, sizeof(input)); //Reading a string from the keyboard

		if (STRINGS_ARE_EQUAL(input, "exit"))
		{
			game_ended = 1;
			return 0x555; //"quit" signals an exit from the client side
		}

		if (input_to_cmd(input, cmd_to_server)) // if entered, the command is wrong. try again
		{
			continue;		//try again
		}
		cmd_ready = 1;			// update the sending thread that there is a new message ready
	}
}

/*
Function
------------------------
Description �
Parameters	�
Returns		�
*/
void MainClient(int argc, char *argv[])
{
	DWORD wait_res;
	SOCKADDR_IN clientService;
	HANDLE hThread[3];
	// initialize globals -------------
	game_ended = 0;
	cmd_ready = 0;
	connected = 0;
	client_log = NULL;
	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j < 7; j++)
		{
			board[i][j] = 0;
		}
	}
	// finish initializing globals -------------

    // Initialize Winsock.
    WSADATA wsaData; //Create a WSADATA object called wsaData.
	//The WSADATA structure contains information about the Windows Sockets implementation.

	client_log = fopen(argv[2], "w");
	if (client_log == NULL)
	{
		printf("Failed to open log file for writing\n");
		return (EXIT_ERROR);
	}
	
	//Call WSAStartup and check for errors.
    int iResult = WSAStartup( MAKEWORD(2, 2), &wsaData );
	if (iResult != NO_ERROR) {
		printf("Error at WSAStartup()\n");
		fputs("Error at WSAStartup()\n", client_log);
		return EXIT_ERROR;
	}
	//Call the socket function and return its value to the m_socket variable. 
	// For this application, use the Internet address family, streaming sockets, and the TCP/IP protocol.
	
	// Create a socket.
    m_socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

	// Check for errors to ensure that the socket is a valid socket.
    if ( m_socket == INVALID_SOCKET ) {
        printf( "Error at socket(): %ld\n", WSAGetLastError() );
		fputs("Error at socket(): %ld\n", WSAGetLastError(), client_log);
		WSACleanup();
		return EXIT_ERROR;
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
    clientService.sin_port = htons(4444); //Setting the port to connect to.
	
	/*
		AF_INET is the Internet address family. 
	*/


    // Call the connect function, passing the created socket and the sockaddr_in structure as parameters. 
	// Check for general errors.
	if ( connect( m_socket, (SOCKADDR*) &clientService, sizeof(clientService) ) == SOCKET_ERROR) {
        printf( "Failed to connect.\n" );
		fputs("Failed to connect.\n", client_log);
        WSACleanup();
		exit (EXIT_ERROR);
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
	if (hThread[0] == NULL)
	{
		printf("couldn't create SendDataThread.\n");
		fputs("couldn't create SendDataThread.\n", client_log);
		return EXIT_ERROR;
	}
	
	hThread[1]=CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE) RecvDataThread,
		NULL,
		0,
		NULL
	);
	if (hThread[1] == NULL)
	{
		printf("couldn't create RecvDataThread.\n");
		fputs("couldn't create RecvDataThread.\n", client_log);
		return EXIT_ERROR;
	}
	
	hThread[2] = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)player_input,
		NULL,
		0,
		NULL
	);
	if (hThread[2] == NULL)
	{
		printf("couldn't create player_input.\n");
		fputs("couldn't create player_input.\n", client_log);
		return EXIT_ERROR;
	}


	//WaitForMultipleObjects(3,hThread,FALSE,INFINITE);

	wait_res = WaitForMultipleObjects(3, hThread, FALSE, INFINITE);
	if (wait_res != WAIT_OBJECT_0) {
		printf("Error when waiting for Client's threads!\n");
		fputs("Error when waiting for Client's threads!\n", client_log);
		return EXIT_ERROR;
	}

	TerminateThread(hThread[0],0x555);
	TerminateThread(hThread[1],0x555);
	TerminateThread(hThread[2],0x555);

	CloseHandle(hThread[0]);
	CloseHandle(hThread[1]);
	CloseHandle(hThread[2]);

	closesocket(m_socket);
	fclose(client_log);
	
	WSACleanup();
    
	return;
}