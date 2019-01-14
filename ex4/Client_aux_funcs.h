/*
Authors			- Eli Slavutsky (308882992) & Nisan Tagar (302344031)
Project Name	- Ex4
Description		- Connect 4 - A Server/Client game With human or file mode players
				- Client auxilary functions
*/
#pragma once
#include <stdio.h>
#include <string.h>
#include <winsock2.h>

#ifndef CLIENT_AUX_FUNCS_H
#define CLIENT_AUX_FUNCS_H	
// Defines ---------------------------------------------------------------------

// Global Variables ------------------------------------------------------------


// Function Declarations -------------------------------------------------------
int chk_if_all_digits(char *str);
void cmd_to_action(char *str);
int input_to_cmd(char *input, char *cmd);
void init_board(void);
void get_cmd_from_file(char *input, char *fp);

#endif // SOCKET_EXAMPLE_CLIENT_H