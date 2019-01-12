/*
Authors			- Eli Slavutsky (308882992) & Nisan Tagar (302344031)
Project Name	- Ex4
Description		-
*/

// Includes --------------------------------------------------------------------
#include "Server_Module.h"
#include "Client_Module.h"
#include "SocketSendRecvTools.h"
#include "Client_aux_funcs.h"


 game_ended		=	0;
 cmd_ready		=	0;
 connected		=	0;		// global flag to know if already connected to server
 game_started	=	0;
 my_turn		=	0;
 read_file		=	1;
 //client_log		= NULL;


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
		
		RecvRes = ReceiveString( server_to_client, m_socket );

		if ( RecvRes == TRNS_FAILED )
		{
			printf("Socket error while trying to read data from socket\n");
			fprintf(client_log, "Socket error while trying to read data from socket\n");
			game_ended = 1;
			exit(007);
		}
		else if ( RecvRes == TRNS_DISCONNECTED )
		{
			printf("Server disconnected. Exiting\n");
			fprintf(client_log, "Server disconnected. Exiting\n");
			game_ended = 1;
			return 0x555;
		}
		else
		{
			fprintf(client_log,"Received from server: %s\n", server_to_client);
			cmd_to_action(server_to_client);
		}
		

	}

	return SUCCESS_CODE;
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
		
			SendRes = SendString(cmd_to_server, m_socket);

			if (SendRes == TRNS_FAILED)
			{
				printf("Socket error while trying to write data to socket\n");
				game_ended = 1;
				exit(007);
			}
			else {

				fprintf(client_log, "Sent to Server: %s\n" ,cmd_to_server);
				cmd_ready = 0;
			};
		}
		else		// new command is not ready yet
			continue;
	}
	return SUCCESS_CODE;
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
	char input [MAX_MSG_SIZE];

	printf("Enter User Name\n");

	while (!game_ended)
	{
		if (cmd_ready)	// if cmd_ready == 1 the message has not been send yet so dont get new data from player.
			continue;
		gets_s(input, sizeof(input)); //Reading a string from the keyboard

		if (STRINGS_ARE_EQUAL(input, "exit"))
		{
			game_ended = 1;
			shutdown(m_socket, SD_BOTH);
			return SUCCESS_CODE; //"quit" signals an exit from the client side
		}

		if (input_to_cmd(input, cmd_to_server)) // if entered, the command is wrong. try again
		{
			continue;		//try again
		}
		cmd_ready = 1;			// update the sending thread that there is a new message ready
	}
	return SUCCESS_CODE;
}


static DWORD file_input(LPVOID lpParam)
{
	char input[MAX_MSG_SIZE];
	char *input_file_path = (char*)lpParam;

	input_file = fopen(input_file_path, "r");

	// read the first line which is the user name
	get_cmd_from_file(input, input_file);
	input_to_cmd(input, cmd_to_server);
	cmd_ready = 1;
	
	while (!game_ended && my_turn)
	{
		if (cmd_ready)	// if cmd_ready == 1 the message has not been sent yet so dont get new data from player.
			continue;

		get_cmd_from_file(input, input_file);

		if (STRINGS_ARE_EQUAL(input, "exit"))
		{
			game_ended = 1;
			shutdown(m_socket, SD_BOTH);
			return SUCCESS_CODE; //"quit" signals an exit from the client side
		}

		if (1==input_to_cmd(input, cmd_to_server)) // if entered, the command is wrong. try again
		{
			continue;		//try again
		}
		if (2 == input_to_cmd(input, cmd_to_server)) // if the command is play command return 2
			my_turn = 0;
		cmd_ready = 1;			// update the sending thread that there is a new message ready
	}
	return SUCCESS_CODE;
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
	DWORD wait_res;
	SOCKADDR_IN clientService;
	HANDLE hThread[3];
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);		//This handle allows us to change the console's color
	int port = (int)strtol((argv[3]), NULL, 10);

	init_board(board);	//init board with zero.


    // Initialize Winsock.
    WSADATA wsaData; //Create a WSADATA object called wsaData.
	//The WSADATA structure contains information about the Windows Sockets implementation.

	client_log = fopen(argv[2], "w");
	if (client_log == NULL)
	{
		printf("Failed to open log file for writing\n");
		exit(EXIT_ERROR);
	}
	
	//Call WSAStartup and check for errors.
    int iResult = WSAStartup( MAKEWORD(2, 2), &wsaData );
	if (iResult != NO_ERROR) {
		printf("Failed connecting to server on 127.0.0.1:%d. Exiting\n", port);
		fprintf(client_log, "Failed connecting to server on 127.0.0.1:%d. Exiting\n", port);
		exit(EXIT_ERROR);
	}
	//Call the socket function and return its value to the m_socket variable. 
	// For this application, use the Internet address family, streaming sockets, and the TCP/IP protocol.
	
	// Create a socket.
    m_socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

	// Check for errors to ensure that the socket is a valid socket.
    if ( m_socket == INVALID_SOCKET ) {
		printf("Failed connecting to server on 127.0.0.1:%d. Exiting\n", port);
		fprintf(client_log, "Failed connecting to server on 127.0.0.1:%d. Exiting\n", port);
		WSACleanup();
		exit(EXIT_ERROR);
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
    clientService.sin_port = htons(port); //Setting the port to connect to.
	
	/*
		AF_INET is the Internet address family. 
	*/


    // Call the connect function, passing the created socket and the sockaddr_in structure as parameters. 
	// Check for general errors.
	if ( connect( m_socket, (SOCKADDR*) &clientService, sizeof(clientService) ) == SOCKET_ERROR) {
		printf("Failed connecting to server on 127.0.0.1:%d. Exiting\n", port);
		fprintf(client_log, "Failed connecting to server on 127.0.0.1:%d. Exiting\n", port);
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
	printf("Connected to server on 127.0.0.1:%d\n",port);
	fprintf(client_log, "Connected to server on 127.0.0.1:%d\n", port);


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
		fprintf(client_log, "couldn't create SendDataThread.\n");
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
		fprintf(client_log, "couldn't create RecvDataThread.\n");
		return EXIT_ERROR;
	}
	
	if (strcmp(argv[4], "file") == 0) // if file input
	{
		hThread[2] = CreateThread(
			NULL,
			0,
			(LPTHREAD_START_ROUTINE)file_input,
			argv[5],
			0,
			NULL
			);
			if (hThread[2] == NULL)
			{
				printf("couldn't create file_input thread.\n");
				fprintf(client_log,"couldn't create file_input thread.\n");
				return EXIT_ERROR;
			}
		
	}
	else 
	{

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
			printf("couldn't create player_input thread.\n");
			fprintf(client_log,"couldn't create player_input thread.\n");
			return EXIT_ERROR;
		}
	}

	//WaitForMultipleObjects(3,hThread,FALSE,INFINITE);

	wait_res = WaitForMultipleObjects(3, hThread, FALSE, INFINITE);
	if (wait_res != WAIT_OBJECT_0) {
		printf("Error when waiting for Client's threads!\n");
		fprintf(client_log, "Error when waiting for Client's threads!\n");
		return EXIT_ERROR;
	}

	TerminateThread(hThread[0],0x555);
	TerminateThread(hThread[1],0x555);
	TerminateThread(hThread[2],0x555);

	CloseHandle(hThread[0]);
	CloseHandle(hThread[1]);
	CloseHandle(hThread[2]);
	CloseHandle(hConsole);

	closesocket(m_socket);
	fclose(client_log);

	
	WSACleanup();
    
	return;
}