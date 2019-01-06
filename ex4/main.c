/*
Authors			- Eli Slavutsky (308882992) & Nisan Tagar (302344031)
Project Name	- Ex4
Description		-
*/

// Includes --------------------------------------------------------------------

#include "Server_Module.h";
#include "Client_Module.h";

int main(int argc, char *argv[])
{
	// check that we have all cmd line arguments passed - ex4.exe client/server <logfile> <server port> <input_mode> <input file>
	if (argc != 6) {
		printf("ERROR: Wrong command line usage (ex4.exe client <logfile> <server port> <input_mode> <input file>) \n");
		exit(1);
	}
	if (strcmp(argv[1], "server") == 0){
		MainServer();
	}
	else if (strcmp(argv[1], "client") == 0) {
		MainClient();
	}

}