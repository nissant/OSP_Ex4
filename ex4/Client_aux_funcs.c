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

int input_to_cmd(char *input, char *cmd)
{
	char *tmp_input [MAX_MSG_SIZE];
	char *space_pos=NULL;
	char *str_ptr=NULL;
	strcpy(tmp_input, input);

	if (connected == 0)			//used only after connection to send the user name
	{
		strcpy(cmd, NEW_USER_REQUEST_STR);
		strcat(cmd, ":");
		strcat(cmd, tmp_input);
		connected = 1;			// to not enter the "if" again. only 1st time.
		return 0;
	}

	// check if wrong command--------------------------
	if (*tmp_input != 'p' || *tmp_input != 'm')
	{
		printf("Error: iilegal command\n");
		fputs("Error: iilegal command\n", client_log);
		return 1; // try again
	}
	space_pos = strchr(tmp_input, ' '); // find space
	if (space_pos == NULL)
	{
		printf("Error: iilegal command\n");
		fputs("Error: iilegal command\n", client_log);
		return 1; // try again
	}

	*space_pos = '\0';
	if (strcmpr(tmp_input, "play") != 0 || strcmpr(tmp_input, "message") != 0)
	{
		printf("Error: iilegal command\n");
		fputs("Error: iilegal command\n", client_log);
		return 1; // try again
	}
	str_ptr = space_pos+1;
	if (*str_ptr == '\0')
	{
		printf("Error: iilegal command\n");
		fputs("Error: iilegal command\n", client_log);
		return 1; // try again
	}
	//finished wrong command check --------------------------


	if (*tmp_input == 'p') // if the input is a play command
	{
		if (!chk_if_all_digits(str_ptr))
		{
			printf("Error: iilegal command\n");
			fputs("Error: iilegal command\n", client_log);
			return 1; // try again
		}
		strcpy(cmd, PLAY_REQUEST_STR);
		strcat(cmd, ":");			//put : inside the cmd
		strcat(cmd, str_ptr); //put the number of column inside the cmd
		return 0;
	}
	else if (*tmp_input == 'm') //if the input is a message
	{
		// used functions that creates a command and put in in cm
	}
}

/*
char* find_first_space(char *str)
{
	char *position_of_space;
	position_of_space = str;
	while (position_of_space != ' ')
		position_of_space++;
	return position_of_space;
}
*/

void cmd_to_action(char *str)
{
	int msg_type = 0;
	char params[MAX_MSG_SIZE];
	msg_type = parseMessage(str, params);
	switch (msg_type)
	{

	case NEW_USER_ACCEPTED:
		printf("You are player number %s\n", params);
		break;

	case NEW_USER_DECLINED:
		printf("Request to join was refused\n");
		game_ended = 1;
		break;

	case GAME_STARTED:
		printf("Game is on!\n");
		break;

	case TURN_SWITCH:
		printf("%s's turn\n", params);
		// need to also write to log file
		break;

	case BOARD_VIEW:
		board[*params][*(params + 1)] = *(params + 2);
		PrintBoard(board, GetStdHandle(STD_OUTPUT_HANDLE));
		break;

	case RECEIVE_MESSAGE:
		printf("%s\n", params);
		break;

	case PLAY_ACCEPTED:
		printf("Well played\n");
		break;

	case PLAY_DECLINED:
		printf("Error: %s\n", params);
		break;

	case GAME_ENDED:
		if (strcmp("0",params)==0)
			printf("Game ended. Everybody wins!\n");
		else
			printf("Game ended. The winner is %s!", params);
		break;
	case BAD_MSG


	default:
		return;
		
	}
}

int chk_if_all_digits(char *str)
{
	while (*str != '\0')
	{
		if (0 == isdigit(*str))
			return 0; // not digit
		str++;
	}
	return 1;
}