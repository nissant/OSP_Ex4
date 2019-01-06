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
	// check that we have all cmd line arguments passed - cfg.txt memin.txt memout.txt regout.txt traceinst.txt traceunit.txt
	if (argc != 7) {
		printf("ERROR: Wrong command line usage (cfg.txt memin.txt memout.txt regout.txt traceinst.txt traceunit.txt) \n");
		exit(1);
	}
}