/*
Authors			- Eli Slavutsky (308882992) & Nisan Tagar (302344031)
Project Name	- Ex4
Description		- Connect 4 - A Server/Client game With human or file mode players
				- Main application file
*/

// Includes --------------------------------------------------------------------

#include "Server_Module.h";
#include "Client_Module.h";

int main(int argc, char *argv[])
{
	// Server cmd line arguments: ex4.exe server <logfile> <server port>
	// Client cmd line arguments: ex4.exe client <logfile> <server port> <input_mode> <input file>
	if (argc < 4 || argc > 6) {
		printf("ERROR: Wrong command line usage: \n	Server mode: ex4.exe server <logfile> <server port> \n	Client mode: ex4.exe client <logfile> <server port> <input_mode> <input file> \n");
		exit(1);
	}
	if (strcmp(argv[1], "server") == 0){
		MainServer(argv);
	}
	else if (strcmp(argv[1], "client") == 0) {
		MainClient(argc,argv);
	}

}