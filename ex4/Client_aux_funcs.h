#pragma once
#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#ifndef CLIENT_AUX_FUNCS_H
#define CLIENT_AUX_FUNCS_H	
// Defines ---------------------------------------------------------------------

// Global Variables ------------------------------------------------------------


// Function Declarations -------------------------------------------------------
//char* find_first_space(char *str);

int chk_if_all_digits(char *str);
void cmd_to_action(char *str);
int input_to_cmd(char *input, char *cmd);
void init_board(int board[][7]);
void get_cmd_from_file(char *input, char *fp);

#endif // SOCKET_EXAMPLE_CLIENT_H