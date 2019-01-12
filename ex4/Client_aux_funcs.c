/*
Authors			- Eli Slavutsky (308882992) & Nisan Tagar (302344031)
Project Name	- Ex4
Description		-
*/

// Includes --------------------------------------------------------------------
#include "Server_Module.h"
#include "Client_Module.h"
#include "Client_aux_funcs.h"
#include "SocketSendRecvTools.h"

int input_to_cmd(char *input, char *cmd)
{
	char tmp_input[MAX_MSG_SIZE];
	char *space_pos=NULL;
	char *str_ptr=NULL;
	strcpy(tmp_input, input);

	if (connected == 0)			//used only after connection to send the user name
	{
		strcpy(cmd, NEW_USER_REQUEST_STR);
		strcat(cmd, ":");
		strcat(cmd, tmp_input);
		strcpy(my_name, input);		// to know my name to know if it is my turn.
		connected = 1;			// to not enter the "if" again. only 1st time.
		return 0;
	}

	// check if wrong command--------------------------
	if (*tmp_input != 'p' && *tmp_input != 'm')
	{
		printf("Error: iilegal command\n");
		fprintf(client_log, "Error: iilegal command\n");
		return 1; // try again
	}
	space_pos = strchr(tmp_input, ' '); // find space
	if (space_pos == NULL)
	{
		printf("Error: iilegal command\n");
		fprintf(client_log, "Error: iilegal command\n");
		return 1; // try again
	}

	*space_pos = '\0';
	if (strcmp(tmp_input, "play") != 0 && strcmp(tmp_input, "message") != 0)
	{
		printf("Error: iilegal command\n");
		fprintf(client_log, "Error: iilegal command\n");
		return 1; // try again
	}
	str_ptr = space_pos+1;
	if (*str_ptr == '\0')
	{
		printf("Error: iilegal command\n");
		fprintf(client_log, "Error: iilegal command\n");
		return 1; // try again
	}
	//finished wrong command check --------------------------


	if (*tmp_input == 'p') // if the input is a play command
	{
		if (!chk_if_all_digits(str_ptr))
		{
			printf("Error: iilegal command\n");
			fprintf(client_log, "Error: iilegal command\n");
			return 1; // try again
		}
		strcpy(cmd, PLAY_REQUEST_STR);
		strcat(cmd, ":");			//put : inside the cmd
		strcat(cmd, str_ptr); //put the number of column inside the cmd
		return 2;
	}
	else if (*tmp_input == 'm') //if the input is a message
	{
		insertSemicolon(str_ptr);
		strcpy(cmd, SEND_MESSAGE_STR);
		strcat(cmd, ":");			//put : inside the cmd
		strcat(cmd, str_ptr); //put the number of column inside the cmd
		return 0;
	}
}


void cmd_to_action(char *str)
{
	int msg_type = 0;
	char *pos = NULL;
	char params[MAX_MSG_SIZE];

	msg_type = parseMessage(str, params);
	switch (msg_type)
	{

	case NEW_USER_ACCEPTED:
		printf("You are player number %s\n", params);
		break;

	case NEW_USER_DECLINED:
		printf("Request to join was refused\n");
		shutdown(m_socket, SD_BOTH);
		game_ended = 1;
		break;

	case GAME_STARTED:
		printf("Game is on!\n");
		game_started = 1;
		break;

	case TURN_SWITCH:
		printf("%s's turn\n", params);
		fprintf(client_log, "%s's turn\n", params);
		if (strcmp(params, my_name) == 0) // if it is my turn now
			my_turn = 1;
		else
			my_turn = 0;
		break;

	case BOARD_VIEW:
		board[*params][*(params + 1)] = *(params + 2);
		PrintBoard(board, GetStdHandle(STD_OUTPUT_HANDLE));
		break;

	case RECEIVE_MESSAGE:
		pos = strchr(params, ' ');
		*pos = ':';
		printf("%s\n", params);
		break;

	case PLAY_ACCEPTED:
		printf("Well played\n");
		break;

	case PLAY_DECLINED:
		printf("Error: %s\n", params);
		if (strcmp(params, "illegal move") == 0)
			my_turn = 1;
		break;

	case GAME_ENDED:
		if (strcmp("0",params)==0)
			printf("Game ended. Everybody wins!\n");
		else
			printf("Game ended. The winner is %s!", params);
		break;
	case BAD_MSG:
		printf("Bad Communication message. exiting");
		fprintf(client_log, "Bad Communication message.exiting");

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

void init_board(int board[][7])
{
	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j < 7; j++)
		{
			board[i][j] = 0;
		}
	}
}

void get_cmd_from_file(char *input,char *fp)
{
	fgets(input, MAX_MSG_SIZE, fp);
	trimwhitespace(input);
}