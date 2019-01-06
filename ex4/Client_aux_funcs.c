/*
Authors			- Eli Slavutsky (308882992) & Nisan Tagar (302344031)
Project Name	- Ex4
Description		-
*/

// Includes --------------------------------------------------------------------
#include "Server_Module.h"
#include "Client_Module.h"
#include "Client_aux_funcs.h"
//#include "SocketSendRecvTools.h"

void input_to_cmd(char *input, char *cmd)
{
	char *tmp_input;
	char *space_pos;
	strcpy(tmp_input, input);
	if (connected == 0)			//used only after connection to send the user name
	{
		strcpy(cmd, NEW_USER_REQUEST);
		strcat(cmd, ":");
		strcat(cmd, tmp_input);
		connected = 1;			// to not enter the "if" again. only 1st time.
		return;
	}
	if (*tmp_input == 'p') // if the input is a play command
	{
		space_pos = find_first_space(tmp_input); // find the space pos 
		*space_pos = '\0'; 
		strcpy(cmd, PLAY_REQUEST);
		strcat(cmd, ":");			//put : inside the cmd
		strcat(cmd, (space_pos + 1)); //put the number of column inside the cmd
		return;
	}
	else if (*tmp_input == 'm') //if the input is a message
	{
		// used functions that creates a command and put in in cm
	}
}

char* find_first_space(char *str)
{
	char *position_of_space;
	position_of_space = str;
	while (position_of_space != ' ')
		position_of_space++;
	return position_of_space;
}
