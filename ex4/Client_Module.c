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
			cmd_to_action(server_to_client);
		}
		

	}

	return 0;
}

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

//Sending data to the server
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
		}
		else		// new command is not ready yet
			continue;
	}
}

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

//Sending data to the server
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

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

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

